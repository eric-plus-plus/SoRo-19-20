import ImageTile from '../../../src/ol/ImageTile.js';
import Tile from '../../../src/ol/Tile.js';
import TileQueue from '../../../src/ol/TileQueue.js';
import TileState from '../../../src/ol/TileState.js';
import {defaultImageLoadFunction} from '../../../src/ol/source/Image.js';
import {DROP} from '../../../src/ol/structs/PriorityQueue.js';


describe('ol.TileQueue', function() {

  function addRandomPriorityTiles(tq, num) {
    let i, tile, priority;
    for (i = 0; i < num; i++) {
      tile = new Tile();
      priority = Math.floor(Math.random() * 100);
      tq.elements_.push([tile, '', [0, 0]]);
      tq.priorities_.push(priority);
      tq.queuedElements_[tile.getKey()] = true;
    }
  }

  let tileId = 0;
  function createImageTile(opt_tileLoadFunction) {
    ++tileId;
    const tileCoord = [tileId, tileId, tileId];
    const state = 0; // IDLE
    // The tile queue requires a unique URI for each item added.
    // Browsers still load the resource even if they don't understand
    // the charset.  So we create a unique URI by abusing the charset.
    const src = 'data:image/gif;charset=junk-' + tileId +
        ';base64,R0lGODlhAQABAPAAAP8AAP///' +
        'yH5BAAAAAAALAAAAAABAAEAAAICRAEAOw==';

    const tileLoadFunction = opt_tileLoadFunction ?
      opt_tileLoadFunction : defaultImageLoadFunction;
    return new ImageTile(tileCoord, state, src, null, tileLoadFunction);
  }

  describe('#loadMoreTiles()', function() {
    const noop = function() {};

    it('works when tile queues share tiles', function(done) {
      const q1 = new TileQueue(noop, noop);
      const q2 = new TileQueue(noop, noop);

      const numTiles = 20;
      for (let i = 0; i < numTiles; ++i) {
        const tile = createImageTile();
        q1.enqueue([tile]);
        q2.enqueue([tile]);
      }

      // Initially, both have all tiles.
      expect(q1.getCount()).to.equal(numTiles);
      expect(q2.getCount()).to.equal(numTiles);

      const maxLoading = numTiles / 2;

      // and nothing is loading
      expect(q1.getTilesLoading()).to.equal(0);
      expect(q2.getTilesLoading()).to.equal(0);

      // ask both to load
      q1.loadMoreTiles(maxLoading, maxLoading);
      q2.loadMoreTiles(maxLoading, maxLoading);

      // both tiles will be loading the max
      expect(q1.getTilesLoading()).to.equal(maxLoading);
      expect(q2.getTilesLoading()).to.equal(maxLoading);

      // the second queue will be empty now
      expect(q1.getCount()).to.equal(numTiles - maxLoading);
      expect(q2.getCount()).to.equal(0);

      // let all tiles load
      setTimeout(function() {
        expect(q1.getTilesLoading()).to.equal(0);
        expect(q2.getTilesLoading()).to.equal(0);

        // ask both to load, this should clear q1
        q1.loadMoreTiles(maxLoading, maxLoading);
        q2.loadMoreTiles(maxLoading, maxLoading);

        expect(q1.getCount()).to.equal(0);
        expect(q2.getCount()).to.equal(0);

        done();
      }, 20);

    });

    it('calls #tileChangeCallback_ when all wanted tiles are aborted', function() {
      const tileChangeCallback = sinon.spy();
      const queue = new TileQueue(noop, tileChangeCallback);
      const numTiles = 20;
      for (let i = 0; i < numTiles; ++i) {
        const tile = createImageTile();
        tile.state = TileState.ABORT;
        queue.enqueue([tile]);
      }
      const maxLoading = numTiles / 2;
      queue.loadMoreTiles(maxLoading, maxLoading);
      expect(tileChangeCallback.callCount).to.be(1);
    });

  });

  describe('heapify', function() {
    it('does convert an arbitrary array into a heap', function() {

      const tq = new TileQueue(function() {});
      addRandomPriorityTiles(tq, 100);

      tq.heapify_();
    });
  });

  describe('reprioritize', function() {
    it('does reprioritize the array', function() {

      const tq = new TileQueue(function() {});
      addRandomPriorityTiles(tq, 100);

      tq.heapify_();

      // now reprioritize, changing the priority of 50 tiles and removing the
      // rest

      let i = 0;
      tq.priorityFunction_ = function() {
        if ((i++) % 2 === 0) {
          return DROP;
        }
        return Math.floor(Math.random() * 100);
      };

      tq.reprioritize();
      expect(tq.elements_.length).to.eql(50);
      expect(tq.priorities_.length).to.eql(50);

    });
  });

  describe('tile change event', function() {
    const noop = function() {};

    it('abort queued tiles', function() {
      const tq = new TileQueue(noop, noop);
      const tile = createImageTile();
      expect(tile.hasListener('change')).to.be(false);

      tq.enqueue([tile]);
      expect(tile.hasListener('change')).to.be(true);

      tile.dispose();
      expect(tile.hasListener('change')).to.be(false);
      expect(tile.getState()).to.eql(5); // ABORT
    });

    it('abort loading tiles', function() {
      const tq = new TileQueue(noop, noop);
      const tile = createImageTile(noop);

      tq.enqueue([tile]);
      tq.loadMoreTiles(Infinity, Infinity);
      expect(tq.getTilesLoading()).to.eql(1);
      expect(tile.getState()).to.eql(1); // LOADING

      tile.dispose();
      expect(tq.getTilesLoading()).to.eql(0);
      expect(tile.hasListener('change')).to.be(false);
      expect(tile.getState()).to.eql(5); // ABORT

    });

  });

});
