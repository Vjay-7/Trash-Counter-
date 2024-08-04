// scripts.js

// Firebase configuration
const firebaseConfig = {
    apiKey: "AIzaSyDqoS9NvRMxWaGW8UtnUTt-lgUWFDv9Z1g",
    authDomain: "trashbinpro-c6ff3.firebaseapp.com",
    databaseURL: "https://trashbinpro-c6ff3-default-rtdb.asia-southeast1.firebasedatabase.app/",
    projectId: "trashbinpro-c6ff3",
    storageBucket: "trashbinpro-c6ff3.appspot.com",
    messagingSenderId: "1234567890",
    appId: "1:1234567890:web:abcdefghij"
};

// Initialize Firebase
firebase.initializeApp(firebaseConfig);

// Reference to the database
const db = firebase.database();

function fetchUsers() {
    db.ref('/users').on('value', (snapshot) => {
        const users = snapshot.val();
        const userList = document.getElementById('user-list');
        userList.innerHTML = '';
        for (let uid in users) {
            const user = users[uid];
            const tr = document.createElement('tr');
            tr.innerHTML = `
                <td><span id="name-${uid}">${user.name}</span></td>
                <td>${user.points}</td>
                <td>${uid}</td>
                <td>
                    <button onclick="editUser('${uid}')">Edit</button>
                </td>
            `;
            userList.appendChild(tr);
        }
    });
}

function addUser(uid, name) {
    db.ref('/users/' + uid).set({
        name: name,
        points: 0
    }, (error) => {
        if (error) {
            alert("Failed to add user: " + error.message);
        } else {
            alert("User added successfully!");
        }
    });
}

function editUser(uid) {
    const name = prompt("Enter new name for the user:", document.getElementById('name-' + uid).innerText);
    if (name !== null && name !== "") {
        db.ref('/users/' + uid + '/name').set(name, (error) => {
            if (error) {
                alert("Failed to update user: " + error.message);
            } else {
                alert("User updated successfully!");
            }
        });
    }
}

document.getElementById('add-user-button').addEventListener('click', () => {
    document.getElementById('add-user-modal').style.display = "block";
});

document.getElementById('add-user-form').addEventListener('submit', (e) => {
    e.preventDefault();
    const uid = document.getElementById('new-uid').value.trim();
    const name = document.getElementById('new-name').value.trim();
    if (uid && name) {
        addUser(uid, name);
        document.getElementById('add-user-modal').style.display = "none";
        document.getElementById('add-user-form').reset();
    }
});

document.querySelector('.close').addEventListener('click', () => {
    document.getElementById('add-user-modal').style.display = "none";
});

// Fetch users on page load
fetchUsers();

// Monitor bin status
db.ref('/binStatus').on('value', (snapshot) => {
    const binStatus = snapshot.val();
    const alertDiv = document.getElementById('alert');
    if (binStatus === 'full') {
        alertDiv.style.display = 'block';
        alertDiv.innerHTML = 'Alert: The trash bin is full!';
    } else {
        alertDiv.style.display = 'none';
    }
});
