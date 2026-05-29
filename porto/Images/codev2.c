 //@file codev2.c
 //@brief Programme illustrant un sokoban avec des nouvelle fonctions
 //@author ivan simitzis
 //@version 2.0
 //@date 30/11/2025
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

// Constante
#define TAILLE 12 // 12 par 12
#define HIST_MAX 1000 // nombre max de déplacements dans l'historique

// Utilisateurs
typedef char t_tableau[TAILLE][TAILLE];  // declaration tableau
typedef char t_tabDeplacements[10000];   // tableau pour enregistrer les déplacements

// variable globale
int pos_x_joueur = 0; // position joueur horizontale
int pos_y_joueur = 0; // position joueur verticale
int compteur = 0;     // compteur de déplacements
int echelle = 1;      // 1 = normal, 2 = zoom2, 3 = zoom3

// procedure
void afficher_entete();
int kbhit();
void charger_partie(t_tableau plateau, char fichier[]);
void enregistrer_partie(t_tableau plateau, char fichier[]);
void afficher_plateau(t_tableau plateau);
void zoom2(t_tableau plateau);
void zoom3(t_tableau plateau);
void deplacer(t_tableau plateau, t_tableau copie, char touche);
int compteur_cible(t_tableau plateau);
bool gagne(t_tableau plateau);
void perso_sur_cibles(t_tableau plateau, t_tableau copie);
bool recommencer_partie(char plateau[TAILLE][TAILLE], char copie[TAILLE][TAILLE], char fichier[]);
void gererZoom(char touche);
void copier_plateau(t_tableau src, t_tableau dst);
void trouver_position_joueur(t_tableau plateau);
void caisse_sur_cible(t_tableau plateau, t_tableau copie);
void rafraichir_cibles(t_tableau plateau, t_tableau copie);
void enregistrer_deplacements(t_tabDeplacements t, int nb, char fic[]);
char Affichage(char c);



int main(){
    t_tableau plateau; // represente le plateau
    t_tableau copie;   // represente une copie du plateau initial
    t_tabDeplacements deplacements; // pour enregistrer les déplacements
    t_tableau historique[HIST_MAX]; // historique des déplacements
    int hist_top = 0; // indice de l'historique
    int nb_deplacements = 0; // nombre de déplacements effectués
    char fic[100] = "deplacements.txt"; // fichier pour les déplacements

    // entete + saisie fichier
    afficher_entete();
    char fichier[100];
    printf("Veuillez saisir le nom d'un fichier (.sok)\n");
    scanf("%20s", fichier);

    // initialise le plateau et copie le plateau initial
    charger_partie(plateau, fichier);
    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TAILLE; j++)
            copie[i][j] = plateau[i][j];

    afficher_plateau(plateau);

    // Boucle principale
    while (1) {
        char touche = '\0';
        if (kbhit()) touche = getchar(); // si une touche est pressée

        if (touche == 'r' || touche == 'R') { // touche recommence partie
            if (recommencer_partie(plateau, copie, fichier)) {
                hist_top = 0;
                nb_deplacements = 0;
                continue;
            }
        }

        if (touche == 'x' || touche == 'X') { // touche abandonne
            printf("Vous avez abandonné la partie.\n");
            char sauvegarde;
            printf("Voulez-vous sauvegarder la partie ? (o/n) : ");// propose une sauvegarde
            scanf(" %c", &sauvegarde);
            if (sauvegarde == 'o' || sauvegarde == 'O') {
                char nomFichier[100];
                printf("Nom du fichier pour sauvegarder : "); 
                scanf("%s", nomFichier);
                enregistrer_partie(plateau, nomFichier); // si o sauvegarde sinon quitter
            }
            break;
        }

        // Gestion du zoom avant/arrière
        if (touche == '+' || touche == '-') {
            gererZoom(touche);
            afficher_plateau(plateau);
            continue;
        }

        // deplacement + mise à jour du plateau
        if (touche != '\0') {
            if (hist_top < HIST_MAX) { // cerifie que sa depasse pas le seuil max definis
                copier_plateau(plateau, historique[hist_top]);// sauvegarde les deplacement dans l'historique ddes deplacements
                hist_top++;
            }

            deplacer(plateau, copie, touche);
            caisse_sur_cible(plateau, copie);
            perso_sur_cibles(plateau, copie);
            rafraichir_cibles(plateau, copie);
            afficher_plateau(plateau);

            deplacements[nb_deplacements++] = touche;
            enregistrer_deplacements(deplacements, nb_deplacements, fic);

            if (gagne(plateau)) {
                printf(" Félicitations, vous avez gagné ! \n");
                printf("Nombre total de déplacements : %d\n", compteur);
                break;
            }
        }
    }

    return 0;
}

//  Code principal

void afficher_entete() {
    // affiche tout ce que le joueur doit savoir
    printf("SOKOBAN \n");
    printf("Voici les touches du jeu :\n");
    printf("z : haut\n");
    printf("q : gauche\n");
    printf("d : droite\n");
    printf("s : bas\n");
    printf("r : recommencer la partie\n");
    printf("x : abandonner\n");
    printf("+ : zoom avant\n");
    printf("- : zoom arrière\n\n");
}

void afficher_plateau(t_tableau plateau) { 
    // affiche le plateau avec l’échelle actuelle
    system("clear"); // efface l'écran

    if (echelle == 1) {
        for (int i = 0; i < TAILLE; i++) {
            for (int j = 0; j < TAILLE; j++) {
                printf("%c", Affichage(plateau[i][j]));
            }
            printf("\n");
        }
    } else if (echelle == 2) { // suivant l'echelle de zoom demandée
        zoom2(plateau);
    } else if (echelle == 3) { // suivant l'echelle de zoom demandée
        zoom3(plateau);
    } else { 
        for (int i = 0; i < TAILLE; i++) {
            for (int j = 0; j < TAILLE; j++) {
                printf("%c", Affichage(plateau[i][j]));
            }
            printf("\n");
        }
    }

    printf("\nDéplacements effectués : %d\n", compteur);// montre deplacement effectués
    printf("Cibles restantes : %d\n", compteur_cible(plateau)); // montre cible restantes
}

// Zoom 2 : chaque caractère devient x2 
void zoom2(t_tableau plateau) {
    for (int i = 0; i < TAILLE; i++) {
        char ligneZoom[TAILLE * 2 + 1]; // suivant le zoom
        int k = 0;
        for (int j = 0; j < TAILLE; j++) {
            ligneZoom[k++] = Affichage(plateau[i][j]); // augmente le zoom 2 fois
            ligneZoom[k++] = Affichage(plateau[i][j]);
        }
        ligneZoom[k] = '\0';
        printf("%s\n%s\n", ligneZoom, ligneZoom);
    }
}

// Zoom 3 : chaque caractère devient x3 
void zoom3(t_tableau plateau) {
    for (int i = 0; i < TAILLE; i++) {
        char ligneZoom[TAILLE * 3 + 1]; // suivant le zoom
        int k = 0;
        for (int j = 0; j < TAILLE; j++) {
            ligneZoom[k++] = Affichage(plateau[i][j]);
            ligneZoom[k++] = Affichage(plateau[i][j]);// augmente le zoom 3 fois
            ligneZoom[k++] = Affichage(plateau[i][j]);
        }
        ligneZoom[k] = '\0';
        printf("%s\n%s\n%s\n", ligneZoom, ligneZoom, ligneZoom);
    }
}


void charger_partie(t_tableau plateau, char fichier[]){
    // charge le plateau depuis un fichier et repère le joueur
    FILE * f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne=0 ; ligne<TAILLE ; ligne++){
            for (int colonne=0 ; colonne<TAILLE ; colonne++){
                if (fread(&plateau[ligne][colonne], sizeof(char), 1, f) != 1) {
                    printf("ERREUR LECTURE FICHIER\n");
                    exit(EXIT_FAILURE);
                }
                if (plateau[ligne][colonne] == '@') { // repère le joueur
                    pos_x_joueur = colonne;
                    pos_y_joueur = ligne;
                }
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

void enregistrer_partie(t_tableau plateau, char fichier[]){
    // sauvegarde le plateau dans un fichier
    FILE * f;
    char finDeLigne='\n';

    f = fopen(fichier, "w");
    for (int ligne=0 ; ligne<TAILLE ; ligne++){
        for (int colonne=0 ; colonne<TAILLE ; colonne++){
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}

// Gestion du clavier
int kbhit(){
    // vérifie si une touche est pressée
	int unCaractere=0;
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF){
		ungetc(ch, stdin);
		unCaractere=1;
	} 
	return unCaractere;
}

//  Déplacements
void deplacer(t_tableau plateau, t_tableau copie, char touche) {
    // déplace le joueur et pousse les caisses si possible
    int deltaX = 0;//changement de position selon la touche pressée
    int deltaY = 0;//changement de position selon la touche pressée
    int nouvelleX;//coordonnées de la case devant le joueur
    int nouvelleY;//coordonnées de la case devant le joueur
    int doubleX;//coordonnées de la case derriere le joueur
    int doubleY;//coordonnées de la case derriere le joueur
    char caseDevant;//caractère de la case devant le joueur
    char caseDerriere;//caractère de la case derriere le joueur
    bool joueurEtaitSurCible;// si joueur cible

    // Détermination de la direction
    if (touche == 'z' || touche == 'Z') deltaY = -1;
    else if (touche == 's' || touche == 'S') deltaY = 1;
    else if (touche == 'q' || touche == 'Q') deltaX = -1;
    else if (touche == 'd' || touche == 'D') deltaX = 1;
    else return; // touche non reconnue

    // Calcul des positions
    nouvelleX = pos_x_joueur + deltaX;
    nouvelleY = pos_y_joueur + deltaY;
    doubleX = pos_x_joueur + 2 * deltaX;
    doubleY = pos_y_joueur + 2 * deltaY;

    // Vérification des limites
    if (nouvelleX < 0 || nouvelleX >= TAILLE || nouvelleY < 0 || nouvelleY >= TAILLE) return;

    caseDevant = plateau[nouvelleY][nouvelleX];
    if (doubleX >= 0 && doubleX < TAILLE && doubleY >= 0 && doubleY < TAILLE) caseDerriere = plateau[doubleY][doubleX];
    else caseDerriere = '#';

    // Vérifier si le joueur était sur une cible
    joueurEtaitSurCible = (copie[pos_y_joueur][pos_x_joueur] == '.');

    // Déplacement simple
    if (caseDevant == ' ' || caseDevant == '.') {
        plateau[pos_y_joueur][pos_x_joueur] = joueurEtaitSurCible ? '.' : ' ';
        plateau[nouvelleY][nouvelleX] = (copie[nouvelleY][nouvelleX] == '.') ? '+' : '@';
        pos_x_joueur = nouvelleX;
        pos_y_joueur = nouvelleY;
        compteur++;
    }
    // Pousser une caisse
    else if ((caseDevant == '$' || caseDevant == '*') && (caseDerriere == ' ' || caseDerriere == '.')) {
        plateau[pos_y_joueur][pos_x_joueur] = joueurEtaitSurCible ? '.' : ' ';
        plateau[nouvelleY][nouvelleX] = (copie[nouvelleY][nouvelleX] == '.') ? '+' : '@';
        plateau[doubleY][doubleX] = (copie[doubleY][doubleX] == '.') ? '*' : '$';
        pos_x_joueur = nouvelleX;
        pos_y_joueur = nouvelleY;
        compteur++;
    }
}

//  Gestion des cibles
int compteur_cible(t_tableau plateau) {
    // compte le nombre de cibles vides
    int nb_cibles = 0;
    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TAILLE; j++)
            if (plateau[i][j] == '.') nb_cibles++;
    return nb_cibles;
}

bool gagne(t_tableau plateau) {
    // vérifie si toutes les cibles sont couvertes
    return compteur_cible(plateau) == 0;
}

void perso_sur_cibles(t_tableau plateau, t_tableau copie) {
    // remet le joueur sur la cible si nécessaire
    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TAILLE; j++)
            if (copie[i][j] == '.' && plateau[i][j] == ' ')
                plateau[i][j] = '.';
}

bool recommencer_partie(char plateau[TAILLE][TAILLE], char copie[TAILLE][TAILLE], char fichier[]) {
    // demande confirmation et recharge le plateau initial
    char confirmation;
    printf("Veux tu vraiment recommencer la partie ? (o/n) : ");
    scanf(" %c", &confirmation);
    if (confirmation == 'o' || confirmation == 'O') {
        charger_partie(plateau, fichier);
        for (int i = 0; i < TAILLE; i++)
            for (int j = 0; j < TAILLE; j++)
                copie[i][j] = plateau[i][j];
        compteur = 0;
        afficher_plateau(plateau);
        return true;
    }
    return false;
}

void gererZoom(char touche) {
    // ajuste l'échelle du plateau en fonction de zoom demandée
    if (touche == '+' && echelle < 3) echelle++;
    else if (touche == '-' && echelle > 1) echelle--;
}


void copier_plateau(t_tableau src, t_tableau dst) {
    // copie le plateau src dans dst
    for (int i = 0; i < TAILLE; i++)  
        for (int j = 0; j < TAILLE; j++)
            dst[i][j] = src[i][j]; // pour la sauvegarde des deplacements
}

void trouver_position_joueur(t_tableau plateau) {
    // cherche le joueur sur le plateau
    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TAILLE; j++)
            if (plateau[i][j] == '@' || plateau[i][j] == '+') { // au cas si @ sur cible
                pos_x_joueur = j;
                pos_y_joueur = i;
                return;
            }
}

void caisse_sur_cible(t_tableau plateau, t_tableau copie) {
    // marque les caisses sur les cibles avec '*'
    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TAILLE; j++)
            if (copie[i][j] == '.' && plateau[i][j] == '$')
                plateau[i][j] = '*';
}

void rafraichir_cibles(t_tableau plateau, t_tableau copie) {
    // permet d'eviter les bug sir @ passe sur cible
    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TAILLE; j++)
            if (copie[i][j] == '.' && plateau[i][j] == ' ')
                plateau[i][j] = '.';
}

void enregistrer_deplacements(t_tabDeplacements t, int nb, char fic[]) {
    // sauvegarde l'historique des déplacements dans un fichier
    FILE *f = fopen(fic, "w");
    if (!f) return;
    fwrite(t, sizeof(char), nb, f);
    fclose(f);
}

char Affichage(char c) { 
    if (c == '*') return '$';// change les * de deplace en $
    if (c == '+') return '@'; //change les + de deplace en @
    return c;
}
