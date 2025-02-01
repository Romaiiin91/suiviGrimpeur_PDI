

function envoi() {
    console.log("click sur envoyer");
    $.get("action.cgi", {data: "toto", dir: "N"});
}

function updatePrecisionValue(value) {
    document.getElementById('precision-value').textContent = value;
}
function updateZoomValue(value) {
    document.getElementById('zoom-value').textContent = value;
}

function up() { 
    console.log("move up"); 
    const precisionValue = document.getElementById('precision').value;
    $.get("action.cgi", {move: "up", prec: precisionValue}); 
}
function down() {    
    console.log("move down"); 
    const precisionValue = document.getElementById('precision').value;
    $.get("action.cgi", {move: "down", prec: precisionValue}); 
}
function left() {    
    console.log("move left");
    const precisionValue = document.getElementById('precision').value;
    $.get("action.cgi", {move: "left", prec: precisionValue}); 
}
function right() {   
    console.log("move right");
    const precisionValue = document.getElementById('precision').value; 
    $.get("action.cgi", {move: "right", prec: precisionValue}); 
}
function zoomUpdate() {  
    console.log("zoom Update"); 
    const zoomValue = document.getElementById('zoom').value;
    console.log("zoomValue :", zoomValue);
    $.get("action.cgi", {zoom: zoomValue}); 
}
// function zoomOut() { 
//     console.log("zoom out"); 
//     $.get("action.cgi", {zoom: "-500"}); 
// }
function startRecord() { 
    console.log("record on");
    const nom = document.getElementById("nom").value;
    const prenom = document.getElementById("prenom").value;
    const dropdown = document.getElementById("dropdown");
    $.get("action.cgi", {reco: "on", nom: nom, prenom: prenom, voie: dropdown.value});
}
function stopRecord() { console.log("record off"); $.get("action.cgi", {reco: "off"}); }
function startDetection() { 
    console.log("detection on"); 
    $.get("action.cgi", {detc: "on"}); 
}
function stopDetection() { console.log("detection off"); $.get("action.cgi", {detc: "off"}); }

// Charger et afficher les options
function loadOptions() {
    // Clear existing options
    const dropdown = document.getElementById("dropdown");
    dropdown.innerHTML = ''; // Reset the dropdown content

    // Fetch JSON data
    $.getJSON("positionsEnregistrees.json", {t:Math.random()}, function (data) {
        for (let key in data) {
            const option = document.createElement("option");
            option.value = key;
            option.textContent = `Voie ${key}`;
            dropdown.appendChild(option);
        }
    }).fail(function () {
        console.error("Erreur de chargement des options.");
    });
}
function addRoute() {
    // Demander un numéro à l'utilisateur
    const number = prompt("Entrez un numéro de voie :");

    // Vérifier si l'utilisateur a entré quelque chose
    if (number !== null && number.trim() !== "") {
        console.log("Numéro saisi :", number);
        $.get("action.cgi", { rout: "add", id: number}, function(rep){
            console.log("MAJ du json");
            loadOptions();

        });

    } else alert("Action annulée ou numéro invalide.");
}
function removeRoute() {
    const dropdown = document.getElementById("dropdown");
    const selectedKey = dropdown.value;
    console.log(`Voie selectionnee : ${selectedKey}`);
    $.get("action.cgi", {rout: "rem", id: selectedKey });

    // acknowledgment 
}
function showRoute() {
    const dropdown = document.getElementById("dropdown");
    const selectedKey = dropdown.value;
    console.log(`Voie selectionnee : ${selectedKey}`);
    $.get("action.cgi", {rout:"shw", id: selectedKey});
}

// Charger les options au démarrage
document.addEventListener("DOMContentLoaded", loadOptions);

