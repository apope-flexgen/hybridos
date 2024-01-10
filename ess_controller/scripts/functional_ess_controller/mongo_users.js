// username: flexgen
// password: flexgen1A!
const documents = [
    // BEGIN USER BLOCK
    { "_id" : ObjectId("622b5ed4e16949007908052e"), "username" : "flexgen", "password" : "$2a$10$EXlIC1bWkorwVBqEPV.ze..CbBhkJJ/JVWlK0FBkMLHfcyyVWX.36", "role" : "developer", "password_updated" : ISODate("2022-03-11T14:38:12.445Z"), "__v" : 0 }
    // END USER BLOCK
];
db.users.insertMany(documents);