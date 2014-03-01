var Levelup = require('levelup');
var Sublevel = require('level-sublevel');
var mkdirp = require ('mkdirp');
var spawn = require('child_process').spawn;
var fs = require('fs');
var path = require('path');
var assert = require('assert');

var dbdir = '/tmp/levelfs-testdb';
var mntdir = '/tmp/levelfs-testmnt';
var db = Sublevel(Levelup(dbdir));
mkdirp(mntdir);

function test() {
  var foodb = db.sublevel('foo');
  var bardb = db.sublevel('bar');

  foodb.put('foo1', 'foobar');
  bardb.put('bar1', 'barfoo');

  db.close(mount);
}

function mount() {
  console.log(__dirname);
  var lfs = spawn(path.resolve(__dirname, '..', 'levelfs'), ["-f", dbdir, mntdir]);
  lfs.stdout.pipe(process.stdout);
  lfs.stderr.pipe(process.stderr);

  setTimeout(function(){
    foo1 = fs.readFileSync(path.join(mntdir, 'foo', 'foo1'));
    assert(foo1.toString() == 'foobar');
    bar1 = fs.readFileSync(path.join(mntdir, 'bar', 'bar1'));
    assert(bar1.toString() == 'barfoo');

    var umount = spawn('unmount', [mntdir]);
    umount.stdout.pipe(process.stdout);
    umount.stderr.pipe(process.stderr);
    console.log('PASS');
  }, 1000);

}

test();
