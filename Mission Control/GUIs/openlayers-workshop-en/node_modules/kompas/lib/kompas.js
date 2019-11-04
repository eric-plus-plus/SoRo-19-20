const emitter = require('component-emitter');

module.exports = tracker;

const RAD_PER_DEG = Math.PI / 180;

function toRad(deg) {
  return deg * RAD_PER_DEG;
}

function toDeg(rad) {
  return rad / RAD_PER_DEG;
}

function compassHeading({ alpha, beta, gamma }) {
  if (typeof alpha !== 'number' || typeof beta !== 'number' || typeof gamma !== 'number') {
    return;
  }

  const _x = toRad(beta);
  const _y = toRad(gamma);
  const _z = toRad(alpha);

  const sX = Math.sin(_x);
  const sY = Math.sin(_y);
  const sZ = Math.sin(_z);

  // const cX = Math.cos(_x);
  const cY = Math.cos(_y);
  const cZ = Math.cos(_z);

  const Vx = - cZ * sY - sZ * sX * cY;
  const Vy = - sZ * sY + cZ * sX * cY;

  // Calculate compass heading
  let heading = Math.atan( Vx / Vy );

  // Convert from half unit circle to whole unit circle
  if (Vy < 0) {
    heading += Math.PI;
  } else if (Vx < 0) {
    heading += 2 * Math.PI;
  }

  return toDeg(heading);
}

function tracker({ calculate = true } = {}) {
  let watching = false;
  let lastHeading;
  const DO_EVENT = 'ondeviceorientationabsolute' in window ?
    'deviceorientationabsolute' :
    'deviceorientation';

  const self = {
    watch,
    clear
  };

  function onDeviceOrientation(ev) {
    let heading;
    if ('compassHeading' in ev) {
      heading = ev.compassHeading;
    } else if ('webkitCompassHeading' in ev) {
      heading = ev.webkitCompassHeading;
    } else if (calculate && ev.absolute) {
      heading = compassHeading(ev);
    }
    if (typeof heading === 'number' && !Number.isNaN(heading)) {
      heading = Math.round(heading);
      if (lastHeading !== heading) {
        self.emit('heading', heading);
        lastHeading = heading;
      }
    }
  }

  function watch() {
    if (!watching) {
      watching = true;
      window.addEventListener(DO_EVENT, onDeviceOrientation);
    }
    return self;
  }

  function clear() {
    if (watching) {
      window.removeEventListener(DO_EVENT, onDeviceOrientation);
      watching = false;
    }
    return self;
  }

  return emitter(self);
}
