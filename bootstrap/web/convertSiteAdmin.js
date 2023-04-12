// mongo convertSiteAdmin.js

conn = new Mongo('mongodb://127.0.0.1:27017');
db = conn.getDB('hybridos_authentication');

// Not sure where to get this from yet
const CURRENT_VERSION = '1.0.0';

fancyPrint(...Array(2), '-', true);
print('Running Site Admin Document Conversion Script');

function main() {
    var cursor = db.siteadmins.find({});
    if (cursor.hasNext()) {
        var doc = cursor.next();
        if (doc.version) {
            if (doc.version !== CURRENT_VERSION) {
                if(isOlderVersion(doc.version)) {
                    convertDocument(doc);
                } else {
                    print('Are you trying to downgrade?');
                }
            } else {
                print('Version Matched: Assuming Document is Up To Date');
            }
        } else {
            print('No Version Detected, Current Version: ', CURRENT_VERSION);
            convertDocument(doc);
        }
    } else {
        print('No Site Admin Document Exists, document will be made on web_server/web_ui load');
    }
}

function convertDocument(doc) {
    fancyPrint('Converting Document', true);
    print('Version: ', doc.version);
    switch (doc.version) {
        case '1.0.0':
            print('Converting from version 1.0.0');
            break;
        default:
            print('Converting from last known versionless site admin document type');
            print('Format has password fields as general fields, conversion removes those fields and moves them to password field');
            var passwordKeys = Object.keys(doc)
                .filter(key => !key.startsWith('_'))
                .reduce((noMeta, key) => {
                    noMeta[key] = doc[key];
                    return noMeta;
                }, {});

            db.siteadmins.updateOne({_id: doc._id}, {
                $set: {
                    version: CURRENT_VERSION,
                    password: passwordKeys,
                    radius: {
                        is_enabled: false,
                        ip_address: '127.0.0.1',
                        port: 0000,
                        secret_phrase: '',
                        wait_time: 5000
                    }
                },
                $unset: passwordKeys
            })
            break;
    }
}

function isOlderVersion(oldVersion) {
    print('Do Version Check. If you are seeing this then it has not been implemented');
}

function fancyPrint(string, decorate, decoration = '=', lineBreak = false) {
    if (lineBreak) {
        print(decoration.repeat(92));
    } else {
        var spaces = ' '.repeat((32 - string.length) / 2);
        if (decorate) {
            var string = decoration.repeat(30) + spaces + string + spaces + decoration.repeat(30);
        }
    
        print(string);
    } 
}

main();