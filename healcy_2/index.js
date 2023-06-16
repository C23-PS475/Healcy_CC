const functions = require('firebase-functions');
const firebase = require('firebase-admin');
const express = require("express");
const cors = require("cors");
const { User, Register, Artikel, DataRS, Diskusi } = require("./config");

const fs = require('fs');
const app = express();

app.use(express.json());
app.use(cors());

app.get("/", async (req, res) => {
  const snapshot = await User.get();
  const list = snapshot.docs.map((doc) => ({ id: doc.id, ...doc.data() }));
  res.send(list);
});

// REGISTER APLIKASI
app.post("/register", (req, res) => {
  (async () => {
    try {
      const data = {
        name: req.body.name,
        email: req.body.email,
        password: req.body.password
      };
      await Register.doc().set(data);
      return res.json({ status: "success", data: { register: data } });
    } catch (error) {
      console.log(error);
      res.status(500).send({ status: "Failed", msg: error });
    }
  })();
});

// Login Aplikasi
app.post("/login", (req, res) => {
  (async () => {
    try {
      const email = req.body.email;
      const password = req.body.password;
      const query = Register.where("email", "==", email).where("password", "==", password);
      const querySnapshot = await query.get();
      if (querySnapshot.size > 0) {
        const userId = querySnapshot.docs[0].id;
        const token = await firebase.auth().createCustomToken(userId);
        const userData = querySnapshot.docs[0].data();
        userData.token = token;
        
        // Simpan data login termasuk token ke dalam database User
        await User.doc(userId).set(userData);

        res.json({ status: "Login Berhasil", loginResult: userData });
      } else {
        res.json({ status: "Tidak Ditemukan" });
      }
    } catch (error) {
      console.log(error);
      res.status(500).send({ status: "Failed", msg: error });
    }
  })();
});



app.post('/articles', async (req, res) => {
  try {
    // Membaca file JSON
    const jsonData = fs.readFileSync('artikel.json', 'utf-8');
    const articles = JSON.parse(jsonData).articles;

    // Menambahkan setiap artikel ke database
    const addedArticles = [];
    for (const articleData of articles) {
      const { id, linkimage, title, author, content} = articleData;
      await Artikel.doc(id).set({
        linkimage,
        title,
        author,
        content
      });
      addedArticles.push(articleData);
    }

    res.json(addedArticles);
  } catch (error) {
    console.error(error);
    res.status(500).json({ error: 'Gagal menambahkan artikel' });
  }
});


app.get('/articles/:id', async (req, res) => {
  try {
    const authorizationHeader = req.headers.authorization;

    if (!authorizationHeader || !authorizationHeader.startsWith('Bearer')) {
      return res.status(401).json({ error: 'Unauthorized' });
    }

    const token = authorizationHeader.split(' ')[1];
    // Perform token verification or manipulation as needed
    // ...

    const articleId = req.params.id;
    const articleRef = Artikel.doc(articleId);
    const doc = await articleRef.get();

    if (!doc.exists) {
      return res.status(404).json({ error: 'Artikel tidak ditemukan' });
    }

    const articleData = doc.data();
    return res.json(articleData);
  } catch (error) {
    console.error(error);
    res.status(500).json({ error: 'Gagal memperoleh artikel' });
  }
});


app.post("/update", async (req, res) => {
  const id = req.body.id;
  delete req.body.id;
  const data = req.body;
  await User.doc(id).update(data);
  res.send({ msg: "Updated" });
});

app.post("/delete", async (req, res) => {
  const id = req.body.id;
  await User.doc(id).delete();
  res.send({ msg: "Deleted" });
});
//app.listen(4000, () => console.log("Up & RUnning *4000"));
exports.api = functions.region("asia-southeast2").https.onRequest(app);