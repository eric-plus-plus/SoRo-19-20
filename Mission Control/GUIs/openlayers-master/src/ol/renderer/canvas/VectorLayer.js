/**
 * @module ol/renderer/canvas/VectorLayer
 */
import {getUid} from '../../util.js';
import ViewHint from '../../ViewHint.js';
import {buffer, createEmpty, containsExtent, getWidth, intersects as intersectsExtent} from '../../extent.js';
import {fromUserExtent, toUserExtent, getUserProjection, getTransformFromProjections} from '../../proj.js';
import CanvasBuilderGroup from '../../render/canvas/BuilderGroup.js';
import ExecutorGroup, {replayDeclutter} from '../../render/canvas/ExecutorGroup.js';
import CanvasLayerRenderer from './Layer.js';
import {defaultOrder as defaultRenderOrder, getTolerance as getRenderTolerance, getSquaredTolerance as getSquaredRenderTolerance, renderFeature} from '../vector.js';
import {toString as transformToString, makeScale, makeInverse} from '../../transform.js';

/**
 * @classdesc
 * Canvas renderer for vector layers.
 * @api
 */
class CanvasVectorLayerRenderer extends CanvasLayerRenderer {

  /**
   * @param {import("../../layer/Vector.js").default} vectorLayer Vector layer.
   */
  constructor(vectorLayer) {

    super(vectorLayer);

    /** @private */
    this.boundHandleStyleImageChange_ = this.handleStyleImageChange_.bind(this);


    /**
     * @private
     * @type {boolean}
     */
    this.dirty_ = false;

    /**
     * @private
     * @type {number}
     */
    this.renderedRevision_ = -1;

    /**
     * @private
     * @type {number}
     */
    this.renderedResolution_ = NaN;

    /**
     * @private
     * @type {import("../../extent.js").Extent}
     */
    this.renderedExtent_ = createEmpty();

    /**
     * @private
     * @type {function(import("../../Feature.js").default, import("../../Feature.js").default): number|null}
     */
    this.renderedRenderOrder_ = null;

    /**
     * @private
     * @type {import("../../render/canvas/ExecutorGroup").default}
     */
    this.replayGroup_ = null;

    /**
     * A new replay group had to be created by `prepareFrame()`
     * @type {boolean}
     */
    this.replayGroupChanged = true;
  }

  /**
   * @inheritDoc
   */
  useContainer(target, transform, opacity) {
    if (opacity < 1) {
      target = null;
    }
    super.useContainer(target, transform, opacity);
  }

  /**
   * @inheritDoc
   */
  renderFrame(frameState, target) {

    const pixelRatio = frameState.pixelRatio;
    const layerState = frameState.layerStatesArray[frameState.layerIndex];

    // set forward and inverse pixel transforms
    makeScale(this.pixelTransform, 1 / pixelRatio, 1 / pixelRatio);
    makeInverse(this.inversePixelTransform, this.pixelTransform);

    this.useContainer(target, this.pixelTransform, layerState.opacity);
    const context = this.context;
    const canvas = context.canvas;

    const replayGroup = this.replayGroup_;
    if (!replayGroup || replayGroup.isEmpty()) {
      if (!this.containerReused && canvas.width > 0) {
        canvas.width = 0;
      }
      return this.container;
    }

    // resize and clear
    const width = Math.round(frameState.size[0] * pixelRatio);
    const height = Math.round(frameState.size[1] * pixelRatio);
    if (canvas.width != width || canvas.height != height) {
      canvas.width = width;
      canvas.height = height;
      const canvasTransform = transformToString(this.pixelTransform);
      if (canvas.style.transform !== canvasTransform) {
        canvas.style.transform = canvasTransform;
      }
    } else if (!this.containerReused) {
      context.clearRect(0, 0, width, height);
    }

    this.preRender(context, frameState);

    const extent = frameState.extent;
    const viewState = frameState.viewState;
    const projection = viewState.projection;
    const rotation = viewState.rotation;
    const projectionExtent = projection.getExtent();
    const vectorSource = this.getLayer().getSource();

    // clipped rendering if layer extent is set
    let clipped = false;
    if (layerState.extent) {
      const layerExtent = fromUserExtent(layerState.extent, projection);
      clipped = !containsExtent(layerExtent, frameState.extent) && intersectsExtent(layerExtent, frameState.extent);
      if (clipped) {
        this.clip(context, frameState, layerExtent);
      }
    }


    const viewHints = frameState.viewHints;
    const snapToPixel = !(viewHints[ViewHint.ANIMATING] || viewHints[ViewHint.INTERACTING]);

    const transform = this.getRenderTransform(frameState, width, height, 0);
    const declutterReplays = this.getLayer().getDeclutter() ? {} : null;
    replayGroup.execute(context, transform, rotation, snapToPixel, undefined, declutterReplays);

    if (vectorSource.getWrapX() && projection.canWrapX() && !containsExtent(projectionExtent, extent)) {
      let startX = extent[0];
      const worldWidth = getWidth(projectionExtent);
      let world = 0;
      let offsetX;
      while (startX < projectionExtent[0]) {
        --world;
        offsetX = worldWidth * world;
        const transform = this.getRenderTransform(frameState, width, height, offsetX);
        replayGroup.execute(context, transform, rotation, snapToPixel, undefined, declutterReplays);
        startX += worldWidth;
      }
      world = 0;
      startX = extent[2];
      while (startX > projectionExtent[2]) {
        ++world;
        offsetX = worldWidth * world;
        const transform = this.getRenderTransform(frameState, width, height, offsetX);
        replayGroup.execute(context, transform, rotation, snapToPixel, undefined, declutterReplays);
        startX -= worldWidth;
      }
    }
    if (declutterReplays) {
      const viewHints = frameState.viewHints;
      const hifi = !(viewHints[ViewHint.ANIMATING] || viewHints[ViewHint.INTERACTING]);
      replayDeclutter(declutterReplays, context, rotation, 1, hifi, frameState.declutterItems);
    }

    if (clipped) {
      context.restore();
    }

    this.postRender(context, frameState);

    const opacity = layerState.opacity;
    const container = this.container;
    if (opacity !== parseFloat(container.style.opacity)) {
      container.style.opacity = opacity === 1 ? '' : opacity;
    }

    return this.container;
  }

  /**
   * @inheritDoc
   */
  forEachFeatureAtCoordinate(coordinate, frameState, hitTolerance, callback, declutteredFeatures) {
    if (!this.replayGroup_) {
      return undefined;
    } else {
      const resolution = frameState.viewState.resolution;
      const rotation = frameState.viewState.rotation;
      const layer = this.getLayer();
      /** @type {!Object<string, boolean>} */
      const features = {};

      const result = this.replayGroup_.forEachFeatureAtCoordinate(coordinate, resolution, rotation, hitTolerance,
        /**
         * @param {import("../../Feature.js").FeatureLike} feature Feature.
         * @return {?} Callback result.
         */
        function(feature) {
          const key = getUid(feature);
          if (!(key in features)) {
            features[key] = true;
            return callback(feature, layer);
          }
        }, layer.getDeclutter() ? declutteredFeatures : null);

      return result;
    }
  }

  /**
   * @inheritDoc
   */
  handleFontsChanged() {
    const layer = this.getLayer();
    if (layer.getVisible() && this.replayGroup_) {
      layer.changed();
    }
  }

  /**
   * Handle changes in image style state.
   * @param {import("../../events/Event.js").default} event Image style change event.
   * @private
   */
  handleStyleImageChange_(event) {
    this.renderIfReadyAndVisible();
  }

  /**
   * @inheritDoc
   */
  prepareFrame(frameState) {
    const vectorLayer = this.getLayer();
    const vectorSource = vectorLayer.getSource();

    const animating = frameState.viewHints[ViewHint.ANIMATING];
    const interacting = frameState.viewHints[ViewHint.INTERACTING];
    const updateWhileAnimating = vectorLayer.getUpdateWhileAnimating();
    const updateWhileInteracting = vectorLayer.getUpdateWhileInteracting();

    if (!this.dirty_ && (!updateWhileAnimating && animating) ||
        (!updateWhileInteracting && interacting)) {
      return true;
    }

    const frameStateExtent = frameState.extent;
    const viewState = frameState.viewState;
    const projection = viewState.projection;
    const resolution = viewState.resolution;
    const pixelRatio = frameState.pixelRatio;
    const vectorLayerRevision = vectorLayer.getRevision();
    const vectorLayerRenderBuffer = vectorLayer.getRenderBuffer();
    let vectorLayerRenderOrder = vectorLayer.getRenderOrder();

    if (vectorLayerRenderOrder === undefined) {
      vectorLayerRenderOrder = defaultRenderOrder;
    }

    const extent = buffer(frameStateExtent,
      vectorLayerRenderBuffer * resolution);
    const projectionExtent = viewState.projection.getExtent();

    if (vectorSource.getWrapX() && viewState.projection.canWrapX() &&
        !containsExtent(projectionExtent, frameState.extent)) {
      // For the replay group, we need an extent that intersects the real world
      // (-180° to +180°). To support geometries in a coordinate range from -540°
      // to +540°, we add at least 1 world width on each side of the projection
      // extent. If the viewport is wider than the world, we need to add half of
      // the viewport width to make sure we cover the whole viewport.
      const worldWidth = getWidth(projectionExtent);
      const gutter = Math.max(getWidth(extent) / 2, worldWidth);
      extent[0] = projectionExtent[0] - gutter;
      extent[2] = projectionExtent[2] + gutter;
    }

    if (!this.dirty_ &&
        this.renderedResolution_ == resolution &&
        this.renderedRevision_ == vectorLayerRevision &&
        this.renderedRenderOrder_ == vectorLayerRenderOrder &&
        containsExtent(this.renderedExtent_, extent)) {
      this.replayGroupChanged = false;
      return true;
    }

    if (this.replayGroup_) {
      this.replayGroup_.dispose();
    }
    this.replayGroup_ = null;

    this.dirty_ = false;

    const replayGroup = new CanvasBuilderGroup(
      getRenderTolerance(resolution, pixelRatio), extent, resolution,
      pixelRatio, vectorLayer.getDeclutter());

    const userProjection = getUserProjection();
    let userTransform;
    if (userProjection) {
      vectorSource.loadFeatures(toUserExtent(extent, projection), resolution, userProjection);
      userTransform = getTransformFromProjections(userProjection, projection);
    } else {
      vectorSource.loadFeatures(extent, resolution, projection);
    }

    const squaredTolerance = getSquaredRenderTolerance(resolution, pixelRatio);

    /**
     * @param {import("../../Feature.js").default} feature Feature.
     * @this {CanvasVectorLayerRenderer}
     */
    const render = function(feature) {
      let styles;
      const styleFunction = feature.getStyleFunction() || vectorLayer.getStyleFunction();
      if (styleFunction) {
        styles = styleFunction(feature, resolution);
      }
      if (styles) {
        const dirty = this.renderFeature(feature, squaredTolerance, styles, replayGroup, userTransform);
        this.dirty_ = this.dirty_ || dirty;
      }
    }.bind(this);

    const userExtent = toUserExtent(extent, projection);
    if (vectorLayerRenderOrder) {
      /** @type {Array<import("../../Feature.js").default>} */
      const features = [];
      vectorSource.forEachFeatureInExtent(userExtent,
        /**
         * @param {import("../../Feature.js").default} feature Feature.
         */
        function(feature) {
          features.push(feature);
        });
      features.sort(vectorLayerRenderOrder);
      for (let i = 0, ii = features.length; i < ii; ++i) {
        render(features[i]);
      }
    } else {
      vectorSource.forEachFeatureInExtent(userExtent, render);
    }

    const replayGroupInstructions = replayGroup.finish();
    const executorGroup = new ExecutorGroup(extent, resolution,
      pixelRatio, vectorSource.getOverlaps(),
      replayGroupInstructions, vectorLayer.getRenderBuffer());

    this.renderedResolution_ = resolution;
    this.renderedRevision_ = vectorLayerRevision;
    this.renderedRenderOrder_ = vectorLayerRenderOrder;
    this.renderedExtent_ = extent;
    this.replayGroup_ = executorGroup;

    this.replayGroupChanged = true;
    return true;
  }

  /**
   * @param {import("../../Feature.js").default} feature Feature.
   * @param {number} squaredTolerance Squared render tolerance.
   * @param {import("../../style/Style.js").default|Array<import("../../style/Style.js").default>} styles The style or array of styles.
   * @param {import("../../render/canvas/BuilderGroup.js").default} builderGroup Builder group.
   * @param {import("../../proj.js").TransformFunction} opt_transform Transform from user to view projection.
   * @return {boolean} `true` if an image is loading.
   */
  renderFeature(feature, squaredTolerance, styles, builderGroup, opt_transform) {
    if (!styles) {
      return false;
    }
    let loading = false;
    if (Array.isArray(styles)) {
      for (let i = 0, ii = styles.length; i < ii; ++i) {
        loading = renderFeature(
          builderGroup, feature, styles[i], squaredTolerance,
          this.boundHandleStyleImageChange_, opt_transform) || loading;
      }
    } else {
      loading = renderFeature(
        builderGroup, feature, styles, squaredTolerance,
        this.boundHandleStyleImageChange_, opt_transform);
    }
    return loading;
  }
}


export default CanvasVectorLayerRenderer;
