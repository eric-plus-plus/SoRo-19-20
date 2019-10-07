import Feature from '../../../../src/ol/Feature.js';
import LineString from '../../../../src/ol/geom/LineString.js';
import Point from '../../../../src/ol/geom/Point.js';
import Polygon from '../../../../src/ol/geom/Polygon.js';
import {get as getProjection} from '../../../../src/ol/proj.js';
import Cluster from '../../../../src/ol/source/Cluster.js';
import Source from '../../../../src/ol/source/Source.js';
import VectorSource from '../../../../src/ol/source/Vector.js';

describe('ol.source.Cluster', function() {

  describe('constructor', function() {
    it('returns a cluster source', function() {
      const source = new Cluster({
        projection: getProjection('EPSG:4326'),
        source: new VectorSource()
      });
      expect(source).to.be.a(Source);
      expect(source).to.be.a(Cluster);
      expect(source.getDistance()).to.be(20);
    });
  });

  describe('#loadFeatures', function() {
    const extent = [-1, -1, 1, 1];
    const projection = getProjection('EPSG:3857');
    it('clusters a source with point features', function() {
      const source = new Cluster({
        source: new VectorSource({
          features: [
            new Feature(new Point([0, 0])),
            new Feature(new Point([0, 0]))
          ]
        })
      });
      source.loadFeatures(extent, 1, projection);
      expect(source.getFeatures().length).to.be(1);
      expect(source.getFeatures()[0].get('features').length).to.be(2);
    });
    it('clusters with a custom geometryFunction', function() {
      const source = new Cluster({
        geometryFunction: function(feature) {
          const geom = feature.getGeometry();
          if (geom.getType() == 'Point') {
            return geom;
          } else if (geom.getType() == 'Polygon') {
            return geom.getInteriorPoint();
          }
          return null;
        },
        source: new VectorSource({
          features: [
            new Feature(new Point([0, 0])),
            new Feature(new LineString([[0, 0], [1, 1]])),
            new Feature(new Polygon(
              [[[-1, -1], [-1, 1], [1, 1], [1, -1], [-1, -1]]]))
          ]
        })
      });
      source.loadFeatures(extent, 1, projection);
      expect(source.getFeatures().length).to.be(1);
      expect(source.getFeatures()[0].get('features').length).to.be(2);
    });
  });

  describe('#setDistance', function() {
    it('changes the distance value', function() {
      const source = new Cluster({
        distance: 100,
        source: new VectorSource()
      });
      expect(source.getDistance()).to.be(100);
      source.setDistance(10);
      expect(source.getDistance()).to.be(10);
    });
  });

});
