const config = {
  mongodb: {
    url: 'mongodb://localhost:27017',
    databaseName: 'hybridos_authentication',
    options: {
      useNewUrlParser: true, // removes a deprecation warning when connecting
      useUnifiedTopology: true, // removes a deprecating warning when connecting
      //   connectTimeoutMS: 3600000, // increase connection timeout to 1 hour
      //   socketTimeoutMS: 3600000, // increase socket timeout to 1 hour
    },
  },

  // The migrations dir, can be an relative or absolute path. Only edit this when really necessary.
  migrationsDir: './migrate-mongo/migrations/hybridos_authentication',
  changelogCollectionName: 'changelog',
  migrationFileExtension: '.js',

  // Enable the algorithm to create a checksum of the file contents and use that in the comparison to determine
  // if the file should be run.  Requires that scripts are coded to be run multiple times.
  useFileHash: false,
  moduleSystem: 'commonjs',
};

module.exports = config;
