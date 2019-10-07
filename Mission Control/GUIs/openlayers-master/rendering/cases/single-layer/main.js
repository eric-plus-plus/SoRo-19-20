import Map from '../../../src/ol/Map.js';
import View from '../../../src/ol/View.js';
import TileLayer from '../../../src/ol/layer/Tile.js';
import XYZ from '../../../src/ol/source/XYZ.js';

new Map({
  layers: [
    new TileLayer({
      source: new XYZ({
        url: '/data/tiles/satellite/{z}/{x}/{y}.jpg'
      })
    })
  ],
  target: 'map',
  view: new View({
    center: [0, 0],
    zoom: 0
  })
});

render({message: 'A single layer with a XZY source'});
