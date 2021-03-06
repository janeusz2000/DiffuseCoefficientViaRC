const path = require('path');

const simulation = {
  mode : 'development',
  devtool : false,
  entry : './gui/simulation.js',
  output : {filename : 'simulation.js', path : path.resolve(__dirname, 'dist')},
};

const results = {
  mode : 'development',
  devtool : false,
  entry : './gui/results.js',
  output : {filename : 'results.js', path : path.resolve(__dirname, 'dist')},
};

const normalizedResults = {
  mode : 'development',
  devtool : false,
  entry : './gui/normalizedResults.js',
  output : {
    filename : 'normalizedResults.js',
    path : path.resolve(__dirname, 'dist')
  },
};

const preLoader = {
  mode : 'development',
  devtool : false,
  entry : './gui/preLoader.js',
  output : {filename : 'preLoader.js', path : path.resolve(__dirname, 'dist')},
};

const polarData = {
  mode : 'development',
  devtool : false,
  entry : './gui/src/polarPatterns/data.js',
  output : {filename : 'polarData.js', path : path.resolve(__dirname, 'dist')},
};

const polarPatterns = {
  mode : 'development',
  devtool : false,
  entry : './gui/polarPatterns.js',
  output :
      {filename : 'polarPatterns.js', path : path.resolve(__dirname, 'dist')},
};

module.exports = [
  polarData, polarPatterns, simulation, results, normalizedResults, preLoader
];

//
