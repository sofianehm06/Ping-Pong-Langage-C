/* =====================================================================
 *  TENNIS DE TABLE  -  Jeu de Pong en temps reel contre l'ordinateur
 * ---------------------------------------------------------------------
 *  Premier projet de programmation algorithmique.
 *
 *  Principe : une balle (O) rebondit sur le terrain. Tu controles ta
 *  raquette ( | ) a gauche pour la renvoyer. L'ordinateur fait de meme
 *  avec sa raquette a droite. Si tu rates la balle, l'adversaire marque.
 *  Premier a 7 points gagne la partie.
 *
 *  Commandes :
 *     Z (ou Fleche Haut) ........ monter la raquette
 *     S (ou Fleche Bas) ......... descendre la raquette
 *     Q (ou Echap) .............. quitter la partie
 *
 *  ATTENTION : ce jeu utilise <conio.h> et l'API console de Windows
 *  (lecture des touches en temps reel + positionnement du curseur).
 *  Il est donc concu pour WINDOWS (compilateur MinGW / gcc).
 *
 *  Compilation : gcc tennis_de_table.c -o tennis_de_table
 *  Execution   : tennis_de_table.exe
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>     /* _kbhit(), _getch() : lecture clavier non bloquante */
#include <windows.h>   /* console : curseur, position, Sleep                 */

/* --- Dimensions et reglages du terrain ----------------------------- */
#define LARGEUR          56   /* nombre de colonnes du terrain          */
#define HAUTEUR          20   /* nombre de lignes du terrain            */
#define HAUTEUR_RAQUETTE  4   /* taille (en lignes) d'une raquette       */
#define COL_JOUEUR        3   /* colonne de la raquette du joueur        */
#define COL_ORDI    (LARGEUR - 4)  /* colonne de la raquette de l'ordi   */
#define POINTS_GAGNANT    7   /* score qui termine la partie             */

/* --- Acceleration de la balle pendant l'echange -------------------- */
#define VITESSE_DEPART    1.0  /* vitesse au service (cases par image)   */
#define VITESSE_INCREMENT 0.15 /* gain a chaque coup de raquette          */
#define VITESSE_MAX       3.0  /* plafond pour rester jouable             */

/* --- Variables globales decrivant l'etat du jeu -------------------- *
 * (On les met en global pour simplifier, dans un premier projet.)     */
int raquetteJoueurY;   /* ligne du HAUT de la raquette du joueur        */
int raquetteOrdiY;     /* ligne du HAUT de la raquette de l'ordinateur  */
int balleX, balleY;    /* position de la balle                          */
int balleDX, balleDY;  /* direction de la balle (-1, 0 ou +1)           */
double vitesseBalle;   /* vitesse actuelle (cases/image), monte pendant l'echange */
double accumulateur;   /* reste de deplacement reporte d'une image a l'autre */
int scoreJoueur;
int scoreOrdi;

/* Reglages lies a la difficulte. */
int delaiFrame;        /* pause entre deux images (ms) : balle + lente/rapide */
int vitesseOrdi;       /* nombre de cases dont l'ordi se rapproche par image    */
int paresseOrdi;       /* % de chance que l'ordi ne bouge pas (le rend battable) */

HANDLE hConsole;       /* poignee vers la sortie console (pour le curseur) */

/* ===================================================================== */
/*  OUTILS CONSOLE (Windows)                                            */
/* ===================================================================== */

/* Place le curseur a la colonne x, ligne y de la console. */
void placer_curseur(int x, int y) {
    COORD position = { (SHORT) x, (SHORT) y };
    SetConsoleCursorPosition(hConsole, position);
}

/* Cache le curseur clignotant pour un rendu propre. */
void cacher_curseur(void) {
    CONSOLE_CURSOR_INFO info;
    info.dwSize   = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &info);
}

/* Vide les touches restees dans le tampon clavier. */
void vider_clavier(void) {
    while (_kbhit()) {
        _getch();
    }
}

/* ===================================================================== */
/*  OUTILS D'ENTREE (menus, hors jeu en temps reel)                     */
/* ===================================================================== */

/* Lit un entier au clavier de facon sure (re-demande si invalide). */
int lire_entier(void) {
    int valeur;
    int resultat = scanf("%d", &valeur);
    while (resultat != 1) {
        int c;
        if (resultat == EOF) { printf("\n"); exit(0); }
        while ((c = getchar()) != '\n' && c != EOF) { }
        printf("Entree invalide, reessaie : ");
        resultat = scanf("%d", &valeur);
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
    return valeur;
}

/* Pause jusqu'a ce que l'utilisateur appuie sur Entree. */
void appuyer_entree(void) {
    printf("\nAppuie sur Entree pour continuer...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

/* ===================================================================== */
/*  LA BALLE                                                            */
/* ===================================================================== */

/* (Re)place la balle au centre et l'envoie dans une direction donnee. */
void lancer_balle(int direction_x) {
    balleX  = LARGEUR / 2;
    balleY  = HAUTEUR / 2;
    balleDX = direction_x;                    /* -1 (gauche) ou +1 (droite) */
    balleDY = (rand() % 2 == 0) ? -1 : 1;     /* vers le haut ou le bas     */
    vitesseBalle = VITESSE_DEPART;            /* la balle repart lentement  */
    accumulateur = 0.0;
}

/* Accelere la balle (appele a chaque coup de raquette), sans depasser le
 * plafond pour que la partie reste jouable.                            */
void accelerer_balle(void) {
    vitesseBalle += VITESSE_INCREMENT;
    if (vitesseBalle > VITESSE_MAX) {
        vitesseBalle = VITESSE_MAX;
    }
}

/* ===================================================================== */
/*  AFFICHAGE DU TERRAIN                                                 */
/* ===================================================================== */

/* Dessine tout le terrain (score, murs, raquettes, balle) en une fois. */
void dessiner(void) {
    char terrain[HAUTEUR * (LARGEUR + 2) + 16];
    int  idx = 0;
    int  x, y;

    /* Ligne du score (en haut), avec la vitesse actuelle de la balle. */
    placer_curseur(0, 0);
    printf("   TOI : %d     ORDINATEUR : %d     Vitesse x%.1f     (Q = quitter)   \n",
           scoreJoueur, scoreOrdi, vitesseBalle);

    /* Construction du terrain ligne par ligne dans un tampon. */
    for (y = 0; y < HAUTEUR; y++) {
        for (x = 0; x < LARGEUR; x++) {
            char c = ' ';

            /* Murs du haut et du bas (la balle rebondit dessus). */
            if (y == 0 || y == HAUTEUR - 1) {
                c = '=';
            }
            /* Filet pointille au centre. */
            else if (x == LARGEUR / 2 && y % 2 == 0) {
                c = ':';
            }

            /* Raquette du joueur (prioritaire sur le fond). */
            if (x == COL_JOUEUR &&
                y >= raquetteJoueurY && y < raquetteJoueurY + HAUTEUR_RAQUETTE) {
                c = '|';
            }
            /* Raquette de l'ordinateur. */
            if (x == COL_ORDI &&
                y >= raquetteOrdiY && y < raquetteOrdiY + HAUTEUR_RAQUETTE) {
                c = '|';
            }
            /* La balle (prioritaire sur tout le reste). */
            if (x == balleX && y == balleY) {
                c = 'O';
            }

            terrain[idx++] = c;
        }
        if (y < HAUTEUR - 1) {
            terrain[idx++] = '\n';     /* pas de saut de ligne apres la derniere */
        }
    }
    terrain[idx] = '\0';

    placer_curseur(0, 1);
    printf("%s", terrain);

    /* Rappel des commandes sous le terrain. */
    placer_curseur(0, HAUTEUR + 1);
    printf("  Z / Fleche Haut = monter    S / Fleche Bas = descendre ");
}

/* ===================================================================== */
/*  MISE A JOUR : JOUEUR, ORDINATEUR, BALLE                             */
/* ===================================================================== */

/* Empeche une raquette de sortir du terrain. */
void borner_raquette(int *y) {
    if (*y < 1) {
        *y = 1;
    }
    if (*y > HAUTEUR - 1 - HAUTEUR_RAQUETTE) {
        *y = HAUTEUR - 1 - HAUTEUR_RAQUETTE;
    }
}

/* Lit les touches du joueur et deplace sa raquette. Met *enCours a 0
 * si le joueur demande a quitter.                                     */
void gerer_entree_joueur(int *enCours) {
    while (_kbhit()) {
        int touche = _getch();

        if (touche == 0 || touche == 224) {   /* touche speciale = fleche */
            touche = _getch();
            if (touche == 72) raquetteJoueurY--;   /* Fleche Haut */
            else if (touche == 80) raquetteJoueurY++;   /* Fleche Bas  */
        } else {
            switch (touche) {
                case 'z': case 'Z':
                case 'w': case 'W':
                    raquetteJoueurY--;
                    break;
                case 's': case 'S':
                    raquetteJoueurY++;
                    break;
                case 'q': case 'Q': case 27:   /* 27 = touche Echap */
                    *enCours = 0;
                    break;
            }
        }
    }
    borner_raquette(&raquetteJoueurY);
}

/* Deplace la raquette de l'ordinateur pour suivre la balle. */
void gerer_ordinateur(void) {
    int centre, diff, pas;

    /* L'ordi ne reagit que si la balle vient vers lui... */
    if (balleDX <= 0) {
        return;
    }
    /* ...et il est parfois "paresseux" (ce qui le rend battable). */
    if (rand() % 100 < paresseOrdi) {
        return;
    }

    /* Il rapproche le centre de sa raquette de la balle, d'au plus
     * vitesseOrdi cases (sans depasser la balle).                       */
    centre = raquetteOrdiY + HAUTEUR_RAQUETTE / 2;
    diff   = balleY - centre;
    if (diff >  vitesseOrdi) pas =  vitesseOrdi;
    else if (diff < -vitesseOrdi) pas = -vitesseOrdi;
    else pas = diff;

    raquetteOrdiY += pas;
    borner_raquette(&raquetteOrdiY);
}

/* Renvoie la balle avec un angle qui depend de l'endroit touche sur la
 * raquette : moitie HAUTE -> la balle part vers le haut, moitie BASSE ->
 * vers le bas. On evite volontairement le tir parfaitement horizontal,
 * qui pourrait creer un echange sans fin entre deux raquettes alignees. */
int angle_rebond(int balleLigne, int raquetteHaut) {
    int relatif = balleLigne - raquetteHaut;     /* 0 .. HAUTEUR_RAQUETTE-1 */
    if (relatif < HAUTEUR_RAQUETTE / 2) {
        return -1;                               /* moitie haute : vers le haut */
    } else {
        return 1;                                /* moitie basse : vers le bas  */
    }
}

/* Fait avancer la balle d'UNE case, gere les rebonds et le score.
 * (Appelee plusieurs fois par image quand la balle va vite.)
 * Renvoie 1 si un point a ete marque, sinon 0.                        */
int avancer_balle_une_case(void) {
    int nx = balleX + balleDX;
    int ny = balleY + balleDY;

    /* Rebond sur les murs du haut et du bas. */
    if (ny < 1) {
        ny = 1;
        balleDY = -balleDY;
        if (balleDY == 0) balleDY = 1;
    }
    if (ny > HAUTEUR - 2) {
        ny = HAUTEUR - 2;
        balleDY = -balleDY;
        if (balleDY == 0) balleDY = -1;
    }

    /* Rebond sur la raquette du joueur (balle venant de la droite). */
    if (nx == COL_JOUEUR &&
        ny >= raquetteJoueurY && ny < raquetteJoueurY + HAUTEUR_RAQUETTE) {
        balleDX = 1;
        nx = COL_JOUEUR + 1;
        balleDY = angle_rebond(ny, raquetteJoueurY);
        accelerer_balle();                     /* la balle accelere a chaque renvoi */
    }
    /* Rebond sur la raquette de l'ordinateur (balle venant de la gauche). */
    if (nx == COL_ORDI &&
        ny >= raquetteOrdiY && ny < raquetteOrdiY + HAUTEUR_RAQUETTE) {
        balleDX = -1;
        nx = COL_ORDI - 1;
        balleDY = angle_rebond(ny, raquetteOrdiY);
        accelerer_balle();                     /* la balle accelere a chaque renvoi */
    }

    /* Balle sortie a gauche : l'ordinateur marque. */
    if (nx <= 0) {
        scoreOrdi++;
        lancer_balle(1);          /* on relance vers le joueur */
        return 1;
    }
    /* Balle sortie a droite : le joueur marque. */
    if (nx >= LARGEUR - 1) {
        scoreJoueur++;
        lancer_balle(-1);         /* on relance vers l'ordinateur */
        return 1;
    }

    /* Sinon, la balle continue son chemin. */
    balleX = nx;
    balleY = ny;
    return 0;
}

/* ===================================================================== */
/*  MENUS ET ECRANS                                                     */
/* ===================================================================== */

/* Affiche le menu principal et renvoie le choix de l'utilisateur. */
int afficher_menu(void) {
    system("cls");
    printf("=====================================\n");
    printf("        TENNIS DE TABLE  (C)         \n");
    printf("     Jeu de Pong contre l'ordi       \n");
    printf("=====================================\n\n");
    printf("   1. Jouer une partie\n");
    printf("   2. Regles / commandes\n");
    printf("   3. Quitter\n\n");
    printf("Ton choix (1-3) : ");
    return lire_entier();
}

/* Affiche les regles et les commandes. */
void afficher_regles(void) {
    system("cls");
    printf("============== REGLES ===============\n\n");
    printf("Renvoie la balle (O) avec ta raquette ( | )\n");
    printf("a gauche. L'ordinateur defend a droite.\n\n");
    printf("Si la balle sort de ton cote, l'ordi marque.\n");
    printf("Si elle sort de son cote, tu marques.\n");
    printf("Premier a %d points gagne la partie.\n\n", POINTS_GAGNANT);
    printf("Commandes :\n");
    printf("   Z  ou  Fleche Haut  -> monter\n");
    printf("   S  ou  Fleche Bas   -> descendre\n");
    printf("   Q  ou  Echap        -> quitter la partie\n\n");
    printf("Astuce : touche la balle avec le HAUT ou le BAS\n");
    printf("de ta raquette pour lui donner de l'angle !\n");
    appuyer_entree();
}

/* Demande le niveau de difficulte et regle vitesse + paresse de l'ordi. */
void choisir_difficulte(void) {
    int choix;
    system("cls");
    printf("=========== DIFFICULTE ==============\n\n");
    printf("   1. Facile\n");
    printf("   2. Moyen\n");
    printf("   3. Difficile\n\n");
    printf("Ton choix (1-3) : ");
    choix = lire_entier();
    switch (choix) {
        /*               image(ms)  vitesse raquette ordi   paresse(%) */
        case 1:  delaiFrame = 80; vitesseOrdi = 1; paresseOrdi = 22; break; /* Facile */
        case 3:  delaiFrame = 40; vitesseOrdi = 2; paresseOrdi = 4;  break; /* Difficile */
        default: delaiFrame = 55; vitesseOrdi = 2; paresseOrdi = 15; break; /* Moyen */
    }
}

/* ===================================================================== */
/*  UNE PARTIE COMPLETE                                                  */
/* ===================================================================== */

void jouer_partie(void) {
    int enCours = 1;

    choisir_difficulte();

    /* Position de depart des raquettes et de la balle. */
    scoreJoueur = 0;
    scoreOrdi   = 0;
    raquetteJoueurY = (HAUTEUR - HAUTEUR_RAQUETTE) / 2;
    raquetteOrdiY   = (HAUTEUR - HAUTEUR_RAQUETTE) / 2;
    lancer_balle((rand() % 2 == 0) ? -1 : 1);

    /* Petit compte a rebours avant de commencer. */
    system("cls");
    cacher_curseur();
    vider_clavier();
    placer_curseur(0, HAUTEUR / 2);
    printf("        La partie commence... Prepare-toi !");
    Sleep(1200);
    system("cls");

    /* Boucle principale du jeu (une iteration = une image). */
    while (enCours) {
        gerer_entree_joueur(&enCours);
        gerer_ordinateur();

        /* La balle avance d'autant de cases que sa vitesse, mais case par
         * case : on verifie les collisions a chaque pas pour qu'une balle
         * rapide ne traverse pas une raquette.                          */
        int point = 0;
        accumulateur += vitesseBalle;
        while (accumulateur >= 1.0) {
            accumulateur -= 1.0;
            point = avancer_balle_une_case();
            if (point) {              /* point marque : la balle est relancee */
                accumulateur = 0.0;
                break;
            }
        }

        dessiner();

        /* Petite pause apres chaque point marque. */
        if (point) {
            placer_curseur(0, HAUTEUR / 2);
            printf("            -- POINT --            ");
            Sleep(600);
        }

        /* Fin de partie si quelqu'un atteint le score gagnant. */
        if (scoreJoueur >= POINTS_GAGNANT || scoreOrdi >= POINTS_GAGNANT) {
            enCours = 0;
        }

        Sleep(delaiFrame);
    }

    /* Ecran de fin. */
    vider_clavier();
    system("cls");
    printf("\n\n=============================================\n");
    if (scoreJoueur >= POINTS_GAGNANT)
        printf("   VICTOIRE ! Tu as battu l'ordinateur !\n");
    else if (scoreOrdi >= POINTS_GAGNANT)
        printf("   Defaite... L'ordinateur gagne cette fois.\n");
    else
        printf("   Partie interrompue.\n");
    printf("   Score final : TOI %d - %d ORDINATEUR\n", scoreJoueur, scoreOrdi);
    printf("=============================================\n");
    appuyer_entree();
}

/* ===================================================================== */
/*  PROGRAMME PRINCIPAL                                                  */
/* ===================================================================== */

int main(void) {
    int continuer = 1;

    /* Recupere la sortie console et configure l'affichage. */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleOutputCP(65001);      /* accents corrects dans la console */

    /* Initialise le generateur de nombres aleatoires. */
    srand((unsigned int) time(NULL));

    while (continuer) {
        int choix = afficher_menu();
        switch (choix) {
            case 1:  jouer_partie();   break;
            case 2:  afficher_regles(); break;
            case 3:  continuer = 0;    break;
            default:
                printf("Choix invalide.\n");
                Sleep(800);
                break;
        }
    }

    /* On reaffiche le curseur avant de quitter. */
    CONSOLE_CURSOR_INFO info = { 100, TRUE };
    SetConsoleCursorInfo(hConsole, &info);

    system("cls");
    printf("Merci d'avoir joue ! A bientot.\n");
    return 0;
}
