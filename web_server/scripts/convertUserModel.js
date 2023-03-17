conn = new Mongo('mongodb://127.0.0.1:27017');
db = conn.getDB('hybridos_authentication');

// may need to change version here
CURRENT_VERSION = "1.0.0";
print('Running User Document Conversion Script');


function main() {
    var cursor = db.users.find({});
    while (cursor.hasNext()) {
        var doc = cursor.next();
        if (doc.version != CURRENT_VERSION) {
            up(doc);
        }
    }
    if (db.mfas.count() === 0) {
        db.mfas.drop()
    }
    if (db.oldpasswords.count() === 0) {
        db.oldpasswords.drop()
    }
}

/**
 * Upgrades User model by merging in mfas and oldpasswords model
 * @param {object} doc The user document to update
 */
function up(doc) {
    var mfaCursor = db.mfas.find({user_id: doc._id});
    var oldPasswordsCursor = db.oldpasswords.find({user_id: doc._id});
    if (mfaCursor.hasNext()) {
        mfaDoc = mfaCursor.next();
        db.users.update({_id: doc._id}, {$set: {shared_key: mfaDoc.shared_key, mfa_enabled: mfaDoc.mfa_enabled, version: CURRENT_VERSION}});
    } else {
        db.users.update({_id: doc._id}, {$set: {shared_key: null, mfa_enabled: false}});
    }
    if (oldPasswordsCursor.hasNext()) {
        oldPasswordDoc = oldPasswordsCursor.next();
        db.users.update({_id: doc._id}, {$set: {old_passwords: oldPasswordDoc.old_passwords}});
    } else {
        db.users.update({_id: doc._id}, {$set: {old_passwords: []}});
    }
    db.mfas.deleteOne({user_id: doc._id});
    db.oldpasswords.deleteOne({user_id: doc._id});
}

/**
 * Downgrades User model by creating separate mfa and oldpasswords doc
 * @param {object} doc The user document to downgrade
 */
function down(doc) {
    var old_passwords = doc.old_passwords;
    var mfa_enabled = doc.mfa_enabled;
    var shared_key = doc.shared_key;
    db.mfas.insert({user_id: doc._id, shared_key, mfa_enabled});
    db.oldpasswords.insert({user_id: doc._id, old_passwords});
    db.users.update({_id: doc._id}, {$unset: {shared_key: 1, mfa_enabled: 1, old_passwords: 1}});
    db.users.update({_id: doc._id}, {$set: {version: "0.0.1"}})
}

main();