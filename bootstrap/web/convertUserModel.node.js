const { MongoClient } = require('mongodb');
const crypto = require('crypto');
const { authenticator } = require("otplib");
authenticator.options = { crypto };

const CURRENT_VERSION = "1.0.0";
const MONGO_URI = "mongodb://127.0.0.1:27017";

let users; 
let mfas; 
let oldpasswords;

async function up(user) {
    const mfa = await mfas.findOne({ user_id: user._id });
    const oldPassword = await oldpasswords.findOne({ user_id: user._id });
    const filter = { _id: user._id };
    const options = { upsert: false };
    if (mfa) {
        await users.updateOne(filter, { $set: { shared_key: mfa.shared_key, mfa_enabled: mfa.mfa_enabled, version: CURRENT_VERSION } }, options);
    } else {
        const shared_key = authenticator.generateSecret();
        await users.updateOne(filter, { $set: { shared_key: shared_key, mfa_enabled: false, version: CURRENT_VERSION } }, options);
    }

    if (oldPassword) {
        await users.updateOne(filter, { $set: { old_passwords: oldPassword.old_passwords } }, options);
    } else {
        await users.updateOne(filter, { $set: { old_passwords: [] } }, options);
    }

    await mfas.deleteOne({ user_id: user._id });
    await oldpasswords.deleteOne({ user_id: user._id });
}

async function main() {
    const client = new MongoClient(MONGO_URI, {
        useNewUrlParser: true,
        useUnifiedTopology: true,
    });

    try {
        console.log('Running User Document Conversion Script');
        await client.connect();
        const db = client.db('hybridos_authentication');

        users = db.collection("users");
        mfas = db.collection("mfas");
        oldpasswords = db.collection("oldpasswords");

        const cursor = users.find({});
        while (await cursor.hasNext()) {
            const user = await cursor.next();
            if (user.version != CURRENT_VERSION) {
                console.log(`Updating user: ${user._id}`);
                await up(user);
            }
        }

        if (await db.listCollections({ name: 'mfas' }).hasNext() && await mfas.estimatedDocumentCount() === 0) {
            mfas.drop(() => console.log('mfas collection dropped'))
        }

        if (await db.listCollections({ name: 'oldpasswords' }).hasNext() && await oldpasswords.estimatedDocumentCount() === 0) {
            oldpasswords.drop(() => console.log('oldpasswords collection dropped'))
        }

    } catch (e) {
        console.error(e);
    } finally {
        await client.close();
    }
    console.log('Finished User Document Conversion Script');
}

main().catch(console.error);