const POSITIONS_JSON_URL = "data/positionsEnregistrees.json";
const CAMERAS_JSON_URL = "data/cameras.json";
const DIR_CGI = "/cgi-bin/"


function updatePrecisionValue(value) {
    document.getElementById('precision-value').textContent = value;
}
function updateZoomValue(value) {
    document.getElementById('zoom-value').textContent = value;
}

function up() { 
    console.log("move up"); 
    const precisionValue = document.getElementById('precision').value;
    $.get(DIR_CGI + "action.cgi", {move: "up", prec: precisionValue}); 
}
function down() {    
    console.log("move down"); 
    const precisionValue = document.getElementById('precision').value;
    $.get(DIR_CGI + "action.cgi", {move: "down", prec: precisionValue}); 
}
function left() {    
    console.log("move left");
    const precisionValue = document.getElementById('precision').value;
    $.get(DIR_CGI + "action.cgi", {move: "left", prec: precisionValue}); 
}
function right() {   
    console.log("move right");
    const precisionValue = document.getElementById('precision').value; 
    $.get(DIR_CGI + "action.cgi", {move: "right", prec: precisionValue}); 
}
function zoomUpdate() {  
    console.log("zoom Update"); 
    const zoomValue = document.getElementById('zoom').value;
    console.log("zoomValue :", zoomValue);
    $.get(DIR_CGI + "action.cgi", {zoom: zoomValue}); 
}

function startRecord() { 
    console.log("record on");

    nom = document.getElementById("nom").value;
    prenom = document.getElementById("prenom").value;

    nom = nom.trim() == "" ? "nom" : nom;
    prenom = prenom.trim() == "" ? "prenom" : prenom;

    const dropdown = document.getElementById("dropdown");

    $.get(DIR_CGI + "action.cgi", {reco: "on", nom: nom, prenom: prenom, voie: dropdown.value});
}
function stopRecord() { console.log("record off"); $.get(DIR_CGI + "action.cgi", {reco: "off"}); }
function startDetection() { 
    console.log("detection on"); 
    $.get(DIR_CGI + "action.cgi", {detc: "on"}); 
}
function stopDetection() { console.log("detection off"); $.get(DIR_CGI + "action.cgi", {detc: "off"}); }

function startEnregistrement(){
    console.log("enregistrement on");
    nom = document.getElementById("nom").value;
    prenom = document.getElementById("prenom").value;

    nom = nom.trim() == "" ? "nom" : nom;
    prenom = prenom.trim() == "" ? "prenom" : prenom;

    const dropdown = document.getElementById("dropdown");
    voie = dropdown.value;
    voie = voie.trim() == "" ? "0" : voie;
    

    // $.get(DIR_CGI + "action.cgi", {enrg: "on", nom: nom, prenom: prenom, voie: voie});
    $.get(DIR_CGI + "action.cgi", { enrg: "on", nom: nom, prenom: prenom, voie: voie }, function(data) {
        console.log("Réponse du serveur :", data); // Affiche la réponse dans la console
        if (data.success) {
            console.log("Succès: " + data.message);
        } else {
            alert("Erreur: " + data.error);  // Affiche une popup en cas d'erreur
        }
    }, "json").fail(function(jqXHR, textStatus, errorThrown) {
        console.error("Erreur de communication :", textStatus, errorThrown); // Affiche les détails de l'erreur
        console.error("Réponse du serveur :", jqXHR.responseText); // Affiche la réponse brute du serveur
        alert("Erreur de communication avec le serveur");
    });
}
// update video stream
function updateVideoStream() {
    var img = document.getElementById("videoStream");
    if (img) {
        img.src = DIR_CGI + "video.cgi?timestamp=" + new Date().getTime();
    } else {
        console.error("L'élément avec l'ID 'videoStream' n'a pas été trouvé.");
    }
}



function loadOptions(selectedKey = null) {
    // Clear existing options
    const dropdown = document.getElementById("dropdown");
    dropdown.innerHTML = ''; // Reset the dropdown content

    const dropdownFast = document.getElementById("dropdownFast");
    dropdownFast.innerHTML = ''; // Reset the dropdownFast content

    // Fetch JSON data
    $.getJSON(POSITIONS_JSON_URL, {timestamp:new Date().getTime()}, function (data) {
        for (let key in data) {
            const option = document.createElement("option");
            option.value = key;
            option.textContent = `Voie ${key}`;
            dropdown.appendChild(option);

            const optionFast = document.createElement("option");
            optionFast.value = key;
            optionFast.textContent = `${key}`;
            dropdownFast.appendChild(optionFast);
        }

        // Sélectionner la clé spécifiée si elle est fournie
        if (selectedKey !== null) {
            dropdown.value = selectedKey;
            dropdownFast.value = selectedKey;
        }
    }).fail(function () {
        console.error("Erreur de chargement des options.");
    });
}

function addRoute() {
    // Demander un numéro à l'utilisateur
    const number = prompt("Entrez un numéro de voie :");

    // Vérifier si l'utilisateur a entré quelque chose
    if (number !== null && number.trim() !== "" && number > 0 && number < 1000) {
        console.log("Numéro saisi :", number);
        // Vérifier si le numéro existe déjà dans la liste déroulante
        const options = Array.from(document.getElementById("dropdown").options);
        const exists = options.some(option => option.value === number);

        if (exists) {
            
            if (!confirm("Ce numéro de voie existe déjà. Voulez-vous le remplacer ?")) {
                console.log("Action annulée.");
                return;
            }
           
        }


        $.get(DIR_CGI + "action.cgi", { rout: "add", id: number}, function(rep){
            console.log("MAJ du json");
            loadOptions(number); // Passer la clé sélectionnée à loadOptions
        });

    } else alert("Action annulée ou numéro invalide.");
}

function removeRoute() {
    const dropdown = document.getElementById("dropdown");
    const selectedKey = dropdown.value;
    console.log(`Voie selectionnee : ${selectedKey}`);
    $.get(DIR_CGI + "action.cgi", {rout: "rem", id: selectedKey}, function(rep){
        console.log("MAJ du json");
        loadOptions();
    });

    // acknowledgment 
}
function showRoute() {
    const dropdown = document.getElementById("dropdown");
    const selectedKey = dropdown.value;
    console.log(`Voie selectionnee : ${selectedKey}`);
    $.get(DIR_CGI + "action.cgi", {rout:"shw", id: selectedKey}, function(rep){
        console.log("Changement de caméra effectué");
        updateVideoStream();
    });
}

function showRouteFast() {
    const dropdown = document.getElementById("dropdownFast");
    const selectedKey = dropdown.value;
    console.log(`Voie selectionnee : ${selectedKey}`);
    $.get(DIR_CGI + "action.cgi", {rout:"shw", id: selectedKey}, function(rep){
        console.log("Changement de caméra effectué");
        updateVideoStream();
    });

    document.getElementById("dropdown").value = selectedKey;
}

function loadCameras() {
    const dropdownCameras = document.getElementById("cameras");
    dropdownCameras.innerHTML = ''; // Reset the dropdown content

    // Fetch JSON data
    $.getJSON(CAMERAS_JSON_URL, {timestamp: new Date().getTime()}, function(data) {
        for (let key in data) {
            const option = document.createElement("option");
            option.value = key;
            option.textContent = `${key}`;
            dropdownCameras.appendChild(option);
        }


    }).fail(function() {
        console.error("Erreur de chargement des caméras.");
    });
}

function changeCamera() {
    const dropdownCameras = document.getElementById("cameras");
    const selectedKey = dropdownCameras.value;
    console.log(`Camera selectionnee : ${selectedKey}`);
    $.get(DIR_CGI + "action.cgi", {cam: selectedKey}, function(rep){
        console.log("Changement de caméra effectué");
        updateVideoStream();
    });
}
