var serviceAccount = require("./healcy-388714-firebase-adminsdk-3sisr-6d234e7704.json");
const firebase = require('firebase-admin');
  firebase.initializeApp({
    credential: firebase.credential.cert(serviceAccount),
    databaseURL: "https://healcy-388714-default-rtdb.firebaseio.com"
  });
const db = firebase.firestore();

const collections = {
  User: db.collection("Users"),
  Register: db.collection("Register"),
  Artikel: db.collection("Artikel"),
  DataRS: db.collection("DataRS"),
  Diskusi: db.collection('Diskusi')
};

module.exports = collections;
