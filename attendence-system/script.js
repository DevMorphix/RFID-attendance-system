

(async function () {

    let firebaseData = {};

    let attendanceData = document.getElementById('attendanceData');
    
    // fetch data from firebase
    await fetch("https://pro-test-263ea-default-rtdb.asia-southeast1.firebasedatabase.app/attendence.json").then(response => response.json()
        .then(data => firebaseData = data)
        .then((firebaseData) => {
    
            console.log("%cFirebase data fetched successfully", "color: yellow;");
            // console.log(firebaseData);
    
            // convert firebase object to array
            firebaseData = Object.keys(firebaseData).map(key => ({ ...firebaseData[key], key }));
    
            // sort array in descending order
            firebaseData.sort((a, b) => (a.time > b.time) ? -1 : 1);
            
            // process data and generate DOM elements
            firebaseData.forEach(item => {
                console.log(item);

                let row = document.createElement("tr");
                let nameCell = document.createElement("td");
                nameCell.textContent = item.uid;
                
                let deviceCell = document.createElement("td");
                deviceCell.textContent = item.id
                
                let timeCell = document.createElement("td");
                timeCell.textContent = item.time;
                
                let statusCell = document.createElement("td");
                item.status == 0  ? statusCell.textContent = 'Absent' : statusCell.textContent = 'Present';
                
                row.appendChild(nameCell);
                row.appendChild(deviceCell);
                row.appendChild(timeCell);
                row.appendChild(statusCell);

                attendanceData.appendChild(row);


            });
    
        })
    );
    
    })();