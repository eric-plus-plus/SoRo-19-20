import Map from '../src/ol/Map.js';
import View from '../src/ol/View.js';
import {defaults as defaultControls, Attribution} from '../src/ol/control.js';
import TileLayer from '../src/ol/layer/Tile.js';
import OSM from '../src/ol/source/OSM.js';

const attribution = new Attribution({
  collapsible: false
});
const map = new Map({
  layers: [
    new TileLayer({
      source: new OSM()
    })
  ],
  controls: defaultControls({attribution: false}).extend([attribution]),
  target: 'map',
  view: new View({
    center: [0, 0],
    zoom: 2
  })
});

function checkSize() {
  const small = map.getSize()[0] < 600;
  attribution.setCollapsible(small);
  attribution.setCollapsed(small);
}

window.addEventListener('resize', checkSize);
checkSize();
