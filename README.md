# 🏓 Tennis de Table

**Jeu de Pong en temps réel, en langage C, contre l'ordinateur.**

Mini-jeu développé en langage C : une balle rebondit sur le terrain, tu
déplaces ta raquette de haut en bas pour la renvoyer, et l'ordinateur
défend son camp avec sa propre raquette. Premier projet de programmation
algorithmique.

```
   TOI : 2        ORDINATEUR : 1       (Q = quitter)
========================================================
                            :
   |                        :
   |                        :              |
   |                  O     :              |
   |                        :              |
                            :              |
========================================================
  Z / Fleche Haut = monter    S / Fleche Bas = descendre
```

---

## ✨ Fonctionnalités

- **Jeu en temps réel** : la balle se déplace toute seule et rebondit sur
  les murs et les raquettes.
- **Tu contrôles ta raquette `|`** à gauche ; l'ordinateur défend à droite.
- **Effet d'angle** : touche la balle avec la moitié **haute** de ta raquette
  pour la renvoyer vers le haut, avec la moitié **basse** pour la renvoyer
  vers le bas.
- **Balle qui accélère** : à chaque coup de raquette, l'échange va de plus
  en plus vite (la vitesse est affichée en haut de l'écran). Plus le rallye
  dure, plus il devient difficile de rattraper la balle — elle se remet à
  vitesse normale au point suivant.
- **Trois niveaux de difficulté** : la vitesse de la balle et la défense de
  l'ordinateur changent.
- **Affichage ASCII** fluide (le curseur est repositionné à chaque image,
  pas de clignotement).
- **Menu principal** : Jouer / Règles / Quitter. Premier à **7 points**
  gagne la partie.

---

## ⚠️ Prérequis : Windows

Ce jeu lit le clavier en temps réel et anime la console grâce à
`<conio.h>` (`_kbhit`, `_getch`) et à l'**API console de Windows**. Il est
donc prévu pour **Windows** avec un compilateur **MinGW / gcc**.

---

## 🔧 Compilation

Le projet tient dans un seul fichier : `tennis_de_table.c`.

### Avec GCC

```bash
gcc tennis_de_table.c -o tennis_de_table
```

### Avec le Makefile

```bash
mingw32-make          # compile le jeu
mingw32-make run      # compile (si besoin) puis lance le jeu
mingw32-make clean    # supprime l'exécutable
```

> Sur ton installation MinGW, la commande s'appelle `mingw32-make`
> (le `make` tout court n'est pas installé).

---

## ▶️ Lancer le jeu

```bash
tennis_de_table.exe
```

---

## 🎮 Commandes

| Touche                 | Action                  |
|------------------------|-------------------------|
| `Z` ou `Flèche Haut`   | Monter la raquette      |
| `S` ou `Flèche Bas`    | Descendre la raquette   |
| `Q` ou `Échap`         | Quitter la partie       |

> Astuce : touche la balle avec le **haut** ou le **bas** de ta raquette
> pour lui donner de l'angle et surprendre l'ordinateur.

---

## 🧩 Structure du code

Le code est découpé en fonctions courtes, chacune avec un rôle précis :

| Fonction                | Rôle                                                     |
|-------------------------|----------------------------------------------------------|
| `main`                  | Configure la console et lance la boucle du menu.         |
| `afficher_menu`         | Affiche le menu et lit le choix.                         |
| `afficher_regles`       | Affiche les règles et les commandes.                     |
| `choisir_difficulte`    | Règle la vitesse de la balle et l'attention de l'ordi.   |
| `jouer_partie`          | Boucle principale du jeu (une itération = une image).    |
| `dessiner`              | Dessine tout le terrain (score, murs, raquettes, balle). |
| `gerer_entree_joueur`   | Lit les touches et déplace la raquette du joueur.        |
| `gerer_ordinateur`      | Déplace la raquette de l'ordi pour suivre la balle.      |
| `gerer_balle`           | Avance la balle, gère rebonds et score.                  |
| `angle_rebond`          | Calcule l'angle de renvoi selon le point d'impact.       |
| `placer_curseur`        | Positionne le curseur dans la console (API Windows).     |

Notions abordées : variables d'état, boucle de jeu (*game loop*),
coordonnées `x`/`y` et vecteurs de direction, détection de collisions,
lecture clavier non bloquante, et nombres aléatoires (`rand` / `srand`).

---

## 💡 Idées d'améliorations (pour aller plus loin)

- Ajouter un mode **2 joueurs** (un deuxième jeu de touches, ex. `O`/`L`).
- Faire varier la **taille de la raquette** selon la difficulté.
- Mémoriser le **meilleur score** dans un fichier.
- Ajouter un **son** (`Beep`) quand la balle touche une raquette.
