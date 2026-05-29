<?php
// Récupère tous les fichiers .txt
$fichiers = glob("*.txt");

// Crée le dossier de sortie s'il n'existe pas
if (!is_dir("resultats")) {
    mkdir("resultats");
}

// Pour chaque fichier texte
foreach ($fichiers as $fichier) {

    // Lire le fichier ligne par ligne
    $lignes = file($fichier);

    // Nom du fichier HTML de sortie
    $nom_html = "resultats/" . basename($fichier, ".txt") . ".html";

    $sortie = fopen($nom_html, "w");

    //le titre
    fwrite($sortie, "<h1>" . $lignes[0] . "</h1>\n");

    // paragraphe
    for ($i = 1; $i < count($lignes); $i++) {
        fwrite($sortie, "<p>" . $lignes[$i] . "</p>\n");
    }

    fclose($sortie);
}
?>