#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

// Constante
#define TAILLE 12 // 12 par 12
       

// Utilisateurs
typedef char t_tableau[TAILLE][TAILLE];  // declaration tableau

int pos_x_joueur = 0; // position joueur horizontale
int pos_y_joueur = 0; // position joueur verticale
int compteur = 0; // compteur de déplacements

// procedure
void afficher_entete();
int kbhit();
void charger_partie(t_tableau plateau, char fichier[]);
void enregistrer_partie(t_tableau plateau, char fichier[]);
void afficher_plateau(t_tableau plateau);
void deplacer(t_tableau plateau, t_tableau copie, char touche);
int compteur_cible(t_tableau plateau);
bool gagne(t_tableau plateau);
void perso_sur_cibles(t_tableau plateau, t_tableau copie);
bool recommencer_partie(char plateau[12][12], char copie[12][12], char fichier[]);


int main() {
    
    t_tableau plateau; // represente le plateau
    t_tableau copie; // represente une copie du plateau initial

    // entete + saisie fichier
    afficher_entete();
    char fichier[100];
    printf("Veuillez saisir le nom d'un fichier (.sok)\n");
    scanf("%20s", fichier);

    //initialise le plateau et copie le plateau initiale
    charger_partie(plateau, fichier);
    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TAILLE; j++)
            copie[i][j] = plateau[i][j];
   
    afficher_plateau(plateau);

//Boucle qui continue jusqu’à ce que le joueur gagne ou abandonne
 while (1) {
    char touche = '\0';
    if (kbhit()) touche = getchar(); // si une touche est pressé

    if (touche == 'r' || touche == 'R') {  // touche recommence partie
        if (recommencer_partie(plateau, copie, fichier)) {
            continue;
        }
    }       

    if (touche == 'x' || touche == 'X') { // touche abandonne
        printf("Vous avez abandonné la partie.\n");
        char sauvegarde;
        printf("Voulez-vous sauvegarder la partie ? (o/n) : ");// propose une sauveagarde
        scanf(" %c", &sauvegarde);
        if (sauvegarde == 'o' || sauvegarde == 'O') {
            char nomFichier[100];
            printf("Nom du fichier pour sauvegarder : ");
            scanf("%s", nomFichier);
            enregistrer_partie(plateau, nomFichier); // si o sauvegarde sinon quitter
        }
        break;
    }

    // deplacement + mise a joueur plateau
    if (touche != '\0') {
        deplacer(plateau, copie, touche); 
        afficher_plateau(plateau);
        perso_sur_cibles(plateau, copie);

        if (gagne(plateau)) {
            printf(" Félicitations, vous avez gagné ! \n");
            printf("Nombre total de déplacements : %d\n", compteur);
            break;
        }
    }
 }

    return 0;
}


//code principal :
void afficher_entete() {
    // affiche tout ce que le joueur doit savoir
    printf("SOKOBAN \n");

    printf("Voici les touches du jeu :\n");
    printf("z : haut\n");
    printf("q : gauche\n");
    printf("d : droite\n");
    printf("s : bas\n");
    printf("r : recommencer la partie\n");
    printf("x : abandonner\n\n");
}

void afficher_plateau(t_tableau plateau) { 
    system("clear"); // efface l'entete
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            printf("%c",plateau[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    printf("\nDéplacements effectués : %d\n", compteur);// montre deplacement effectués
    printf("Cibles restantes : %d\n", compteur_cible(plateau)); // montre cible restantes
}

void charger_partie(t_tableau plateau, char fichier[]){
    FILE * f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne=0 ; ligne<TAILLE ; ligne++){
            for (int colonne=0 ; colonne<TAILLE ; colonne++){
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
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

int kbhit(){
	// la fonction retourne :
	// 1 si un caractere est present
	// 0 si pas de caractere présent
	int unCaractere=0;
	struct termios oldt, newt;
	int ch;
	int oldf;

	// mettre le terminal en mode non bloquant
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
	ch = getchar();

	// restaurer le mode du terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
 
	if(ch != EOF){
		ungetc(ch, stdin);
		unCaractere=1;
	} 
	return unCaractere;

}

void deplacer(t_tableau plateau, t_tableau copie, char touche) {
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
    if (touche == 'z' || touche == 'Z') {
        deltaY = -1;
    } else if (touche == 's' || touche == 'S') {
        deltaY = 1;
    } else if (touche == 'q' || touche == 'Q') {
        deltaX = -1;
    } else if (touche == 'd' || touche == 'D') {
        deltaX = 1;
    } else {
        return; // touche non reconnue
    }

    // Calcul des positions
    nouvelleX = pos_x_joueur + deltaX;
    nouvelleY = pos_y_joueur + deltaY;
    doubleX = pos_x_joueur + 2 * deltaX;
    doubleY = pos_y_joueur + 2 * deltaY;

    // Vérification des limites
    if (nouvelleX < 0 || nouvelleX >= TAILLE || nouvelleY < 0 || nouvelleY >= TAILLE) {
        return; //Si la case suivante est en dehors du plateau, le joueur ne peut pas bouger
    }

    caseDevant = plateau[nouvelleY][nouvelleX];//contenu de la case où le joueur veut aller
    if (doubleX >= 0 && doubleX < TAILLE && doubleY >= 0 && doubleY < TAILLE) {
        caseDerriere = plateau[doubleY][doubleX];
    } else { //contenu de la case derrière, ou mur (#) si hors plateau
        caseDerriere = '#';
    }

    // Vérifier si le joueur était sur une cible
    joueurEtaitSurCible = (copie[pos_y_joueur][pos_x_joueur] == '.');

    // Déplacement simple
    if (caseDevant == ' ' || caseDevant == '.') {
        plateau[pos_y_joueur][pos_x_joueur] = joueurEtaitSurCible ? '.' : ' ';//Si la case devant est vide ou une cible, le joueur peut se déplacer
        if (copie[nouvelleY][nouvelleX] == '.') {
            plateau[nouvelleY][nouvelleX] = '+';//Met à joueur le plateau  si cible +
        } else {
            plateau[nouvelleY][nouvelleX] = '@';// met a joueur si case normale (@).
        }
        pos_x_joueur = nouvelleX;
        pos_y_joueur = nouvelleY;
        compteur++; // compteur deplacement
    }
    // Pousser une caisse
    //Si la case devant contient une caisse ($ ou *) et que la case derrière est libre ou une cible
    else if ((caseDevant == '$' || caseDevant == '*') && (caseDerriere == ' ' || caseDerriere == '.')) {
        plateau[pos_y_joueur][pos_x_joueur] = joueurEtaitSurCible ? '.' : ' ';
        if (copie[nouvelleY][nouvelleX] == '.') {
            plateau[nouvelleY][nouvelleX] = '+';
        } else {
            plateau[nouvelleY][nouvelleX] = '@'; // joueur bouge
        }
        if (copie[doubleY][doubleX] == '.') {
            plateau[doubleY][doubleX] = '*';
        } else {
            plateau[doubleY][doubleX] = '$'; // caisse poussé d une case
        }
        pos_x_joueur = nouvelleX;
        pos_y_joueur = nouvelleY;
        compteur++; // compteur deplacement
    }
}


int compteur_cible(t_tableau plateau) {
    int nb_cibles = 0; // cible 0
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {// scan tout les case du plateau 
            if (plateau[i][j] == '.') {   // une cible sans caisse 
                nb_cibles++;// augmente duivant nombre cible sur plateau
            }
        }
    }
    return nb_cibles;
}

bool gagne(t_tableau plateau) {
    int nb_cibles = compteur_cible(plateau);// depend du compteur cible
    if (nb_cibles == 0) { // si il n'y a plus de cible restante
        return true;   // La partie est gagnée
    } else {
        return false;  // sinon Il reste des cibles à couvrir
    }
}

void perso_sur_cibles(t_tableau plateau, t_tableau copie) {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            if (copie[i][j] == '.' && plateau[i][j] == ' ') { // verifie si case cible
                plateau[i][j] = '.'; // on restaure la cible
            }
        }
    }
}

bool recommencer_partie(char plateau[12][12], char copie[12][12], char fichier[]) {

    char confirmation;
    printf("Veux tu vraiment recommencer la partie ? (o/n) : ");
    scanf(" %c", &confirmation);  
    if (confirmation == 'o' || confirmation == 'O') { // si touche o pressé
        charger_partie(plateau, fichier);  // recharge le plateau initial
        for (int i = 0; i < 12; i++)
            for (int j = 0; j < 12; j++)
                copie[i][j] = plateau[i][j];  // remettre la copie à joueur
        compteur = 0;  // remise à zéro du compteur
        afficher_plateau(plateau);
        return true;  // la partie recommencée
    }
    return false;
}
