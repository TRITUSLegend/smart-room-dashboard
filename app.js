// Your Firebase config
const firebaseConfig = {
  apiKey: "AIzaSyAFRU-f7abnXqfow8ImPRU4U2LjgNpSiKg",
  authDomain: "home-automation-system-1dbcf.firebaseapp.com",
  databaseURL: "https://home-automation-system-1dbcf-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "home-automation-system-1dbcf",
  storageBucket: "home-automation-system-1dbcf.appspot.com",
  messagingSenderId: "905035575774",
  appId: "1:905035575774:web:9bf8c9d5f2f975f5f2ad4f"
};

// Initialize Firebase
firebase.initializeApp(firebaseConfig);

// Reference to the database path
const db = firebase.database();
const roomRef = db.ref("room1");

// DOM elements
const tempEl = document.getElementById("temperature");
const motionEl = document.getElementById("motion");
const lightToggle = document.getElementById("lightToggle");
const fanToggle = document.getElementById("fanToggle");

// Listen for Firebase data changes
roomRef.on("value", (snapshot) => {
  const data = snapshot.val();
  if (!data) return;

  tempEl.textContent = `${data.temperature ?? "--"} °C`;
  motionEl.textContent = data.motion ? "Detected" : "None";

  lightToggle.checked = data.light === "ON";
  fanToggle.checked = data.fan === "ON";
});

// Send toggle values to Firebase
lightToggle.addEventListener("change", () => {
  const value = lightToggle.checked ? "ON" : "OFF";
  db.ref("room1/light").set(value);
});

fanToggle.addEventListener("change", () => {
  const value = fanToggle.checked ? "ON" : "OFF";
  db.ref("room1/fan").set(value);
});
