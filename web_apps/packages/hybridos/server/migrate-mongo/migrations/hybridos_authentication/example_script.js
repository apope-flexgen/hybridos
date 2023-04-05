module.exports = {
  async up(db, client) {
    console.log('Ran test script for UP.');
    // Add migration code below:
    // return await db
    //   .collection('users')
    //   .updateMany({}, { $set: { test_up: true } });
  },

  async down(db, client) {
    console.log('Ran test script for DOWN.');
    // Add migration code below:
    // return await db
    //   .collection('users')
    //   .updateMany({}, { $set: { test_up: false } });
  },
};
