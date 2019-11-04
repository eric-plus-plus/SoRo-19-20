import 'ol/ol.css';
import Map from 'ol/Map.js';
import View from 'ol/View.js';
import TileLayer from 'ol/layer/Tile.js';
import Feature from 'ol/Feature';
import Point from 'ol/geom/Point';
import VectorLayer from 'ol/layer/Vector';
import {Vector} from 'ol/source';
import {fromLonLat} from 'ol/proj';
import Renderer from 'ol/renderer/webgl/PointsLayer';
import {clamp, lerp} from 'ol/math';
import Stamen from 'ol/source/Stamen';

const features = [];
const vectorSource = new Vector({
  features: [],
  attributions: 'NASA'
});

const oldColor = [180, 140, 140];
const newColor = [255, 80, 80];

const startTime = Date.now() * 0.001;

// hanle input values & events
const minYearInput = document.getElementById('min-year');
const maxYearInput = document.getElementById('max-year');
function updateStatusText() {
  const div = document.getElementById('status');
  div.querySelector('span.min-year').textContent = minYearInput.value;
  div.querySelector('span.max-year').textContent = maxYearInput.value;
}
minYearInput.addEventListener('input', updateStatusText);
minYearInput.addEventListener('change', updateStatusText);
maxYearInput.addEventListener('input', updateStatusText);
maxYearInput.addEventListener('change', updateStatusText);
updateStatusText();

class PointsLayer extends VectorLayer {
  createRenderer() {
    return new Renderer(this, {
      colorCallback: function(feature, vertex, component) {
        // component at index 3 is alpha
        if (component === 3) {
          return 1;
        }

        // color is interpolated based on year
        const ratio = clamp((feature.get('year') - 1800) / (2013 - 1800), 0, 1);
        return lerp(oldColor[component], newColor[component], ratio) / 255;
      },
      sizeCallback: function(feature) {
        return 18 * clamp(feature.get('mass') / 200000, 0, 1) + 8;
      },
      fragmentShader: [
        'precision mediump float;',

        'uniform float u_time;',
        'uniform float u_minYear;',
        'uniform float u_maxYear;',

        'varying vec2 v_texCoord;',
        'varying float v_opacity;',
        'varying vec4 v_color;',

        'void main(void) {',
        '  float impactYear = v_opacity;',

        // filter out pixels if the year is outside of the given range
        '  if (impactYear < u_minYear || v_opacity > u_maxYear) {',
        '    discard;',
        '  }',

        '  vec2 texCoord = v_texCoord * 2.0 - vec2(1.0, 1.0);',
        '  float sqRadius = texCoord.x * texCoord.x + texCoord.y * texCoord.y;',
        '  float value = 2.0 * (1.0 - sqRadius);',
        '  float alpha = smoothstep(0.0, 1.0, value);',

        '  vec3 color = v_color.rgb;',
        '  float period = 8.0;',
        '  color.g *= 2.0 * (1.0 - sqrt(mod(u_time + impactYear * 0.025, period) / period));',

        '  gl_FragColor = vec4(color, v_color.a);',
        '  gl_FragColor.a *= alpha;',
        '  gl_FragColor.rgb *= gl_FragColor.a;',
        '}'
      ].join(' '),
      opacityCallback: function(feature) {
        // here the opacity channel of the vertices is used to store the year of impact
        return feature.get('year');
      },
      uniforms: {
        u_time: function() {
          return Date.now() * 0.001 - startTime;
        },
        u_minYear: function() {
          return parseInt(minYearInput.value);
        },
        u_maxYear: function() {
          return parseInt(maxYearInput.value);
        }
      }
    });
  }
}


function loadData() {
  const client = new XMLHttpRequest();
  client.open('GET', 'data/meteorites.csv');
  client.onload = function() {
    const csv = client.responseText;
    let curIndex;
    let prevIndex = 0;
    while ((curIndex = csv.indexOf('\n', prevIndex)) > 0) {
      const line = csv.substr(prevIndex, curIndex - prevIndex).split(',');
      prevIndex = curIndex + 1;

      // skip header
      if (prevIndex === 0) {
        continue;
      }

      const coords = fromLonLat([parseFloat(line[4]), parseFloat(line[3])]);

      features.push(new Feature({
        mass: parseFloat(line[1]) || 0,
        year: parseInt(line[2]) || 0,
        geometry: new Point(coords)
      }));
    }
    vectorSource.addFeatures(features);
  };
  client.send();
}

loadData();

const map = new Map({
  target: 'map-container',
  layers: [
    new TileLayer({
      source: new Stamen({
        layer: 'toner'
      })
    }),
    new PointsLayer({
      source: vectorSource
    })
  ],
  view: new View({
    center: [0, 0],
    zoom: 2
  })
});

// animate the map
function animate() {
  map.render();
  window.requestAnimationFrame(animate);
}
animate();
