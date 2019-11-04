import fs from 'fs';
import path from 'path';
import {foo} from './mod';

function main(paths) {
  return Promise.all(paths.map(function(name) {
    return new Promise(function(resolve, reject) {
      fs.readFile(name, function(err, data) {
        if (err) {
          reject(err);
        } else {
          resolve(String(data));
        }
      });
    });
  }));
}

if (require.main === module) {
  process.stdout.write(foo());

  const paths = ['main.js', '.eslintrc'].map(function(name) {
    return path.join(__dirname, name);
  });
  main(paths).then(function(contents) {
    contents.forEach(function(content, index) {
      process.stdout.write(paths[index] + '\n' + content + '\n');
    });
  }).catch(function(err) {
    process.stderr.write('Failed: ' + err.message + '\n');
  });
}
