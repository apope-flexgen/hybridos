/* eslint-disable no-console */
const mongoose = require('mongoose');

// this database reference and schema must be the same as what's in the rest of this repo.
// this is a quick and dirty loader so I'm not going to figure out how to reference the
// db that the rest of the repo is using!

mongoose.connect('mongodb://localhost:27017/hybridos_authentication', { useNewUrlParser: true, useUnifiedTopology: true });
const db = mongoose.connection;

db.on('error', console.error.bind(console, 'connection error:'));

db.once('open', () => {
    const UserSchema = mongoose.Schema({
        username: {
            type: String,
            required: true,
            unique: true,
        },
        password: {
            type: String,
            required: true,
        },
        role: {
            type: String,
            required: true,
        },
    });

    const User = mongoose.model('User', UserSchema, 'users');

    User.countDocuments({}, (err, count) => {
        if (err) console.log('loadDefaultUser error:', err);
        if (count === 0) {
            console.log('+++++++ Zero users in mongodb database, creating default user');
            const defaultAdmin = new User({ username: 'developer', password: '$2b$10$VelSf1KjzVYbA1WFxv6q.u7kOOjVAEFN1KQlC9KG1/q4Fe5IPv9xG', role: 'developer' });
            defaultAdmin.save((error, user) => {
                if (error) {
                    console.log(`\nERROR: ${defaultAdmin.username} not saved. error: ${error}\n`);
                    process.exit();
                }
                console.log(`\nSUCCESS: ${user.username} saved to users collection.\n`);
                process.exit();
            });
        } else {
            console.log(`+++++++ ${count} existing users in MongoDB database`);
        }
    });
});
