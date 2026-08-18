# mod-rotation

*[English version: README.md](README.md)*

Module AzerothCore (compatible fork **liyunfan1223/azerothcore-wotlk**, branche
Playerbot) : une rotation « un bouton » déclenchée par un sort placé sur la
barre d'action. **1 appui = 1 action** — chaque pression lance l'aptitude la
plus prioritaire actuellement disponible.

**Toutes les classes et les 30 spécialisations** sont couvertes. La
spécialisation est détectée automatiquement (arbre de talents dominant), les
rotations suivent les guides Wowhead WotLK Classic, et le module résout le
**rang maximum connu** de chaque sort : il fonctionne à tous les niveaux.

## Etat des tests

⚠️ **Seules quelques classes ont été testées à haut niveau** pour l'instant
(notamment le Guerrier Fureur). Les autres spécialisations sont implémentées
d'après les guides Wowhead mais pas encore éprouvées en jeu — les rapports de
bug et corrections sont les bienvenus (voir Contribuer ci-dessous).

## Contribuer

**Merci de soumettre toute modification de ce module sous forme de pull
request sur ce dépôt**, afin que les corrections et améliorations profitent à
tous au lieu de se disperser dans des forks privés.

Note : il s'agit de la politique de contribution du projet, pas d'une clause
de licence — la GPL-2.0 autorise légalement les forks et copies modifiées (la
publication des sources n'est requise qu'en cas de distribution des
binaires). La pull request est simplement la manière conviviale de partager
ses améliorations avec la communauté.

## Le sort déclencheur

Par défaut : **47322 « Dragonblight Test Spell 02 »**, un sort de test Blizzard
inutilisé, présent dans le `Spell.dbc` du client 3.3.5a : instantané, coût nul,
aucune recharge, GCD 0, un seul effet *Dummy* inoffensif. Tous les joueurs
l'apprennent à la connexion (`Rotation.AutoLearn = 1`).

Le renommage en « Rotation » (et l'icône) se fait côté client par un mini-patch
MPQ : éditer le nom du sort 47322 dans `DBFilesClient\Spell.dbc` (WDBX Editor,
Spell Editor de stoneharry…) et livrer le fichier dans un `patch-4.MPQ` /
`patch-frFR-4.MPQ`. Purement cosmétique : le module fonctionne sans.

## Rotations implémentées

Conditions générales : les coûts (mana/rage/énergie/runes), recharges, GCD et
portées sont vérifiés avant chaque cast — si une aptitude n'est pas disponible,
la suivante de la liste est essayée. « AoE » = au moins `Rotation.AoE.Threshold`
ennemis au corps à corps (défaut : 2). Les soigneurs soignent la **cible
amicale sélectionnée** (ou eux-mêmes) selon les seuils `Rotation.Heal.*`, et
font des dégâts légers si personne n'est blessé.

### Guerrier
- **Armes** : Pourfendre → Exécution (proc Mort soudaine ou <20 %) → Fulgurance (proc Goût du sang) → [AoE : Frappes circulaires, Tempête de lames] → Frappe mortelle → Heurtoir → Frappe héroïque/Enchaînement (rage > seuil)
- **Fureur** : Soif de sang → Tourbillon (AoE) → Heurtoir (proc Déferlement sanguin) → Pourfendre → Exécution (<20 %) → Frappe héroïque/Enchaînement
- **Protection** : Blocage de bouclier → Heurt de bouclier → Vengeance → Onde de choc → [AoE : Coup de tonnerre] → Dévaster → Frappe héroïque/Enchaînement

### Paladin
- **Vindicte** : sceau entretenu (Vengeance/Corruption selon faction) ; Marteau de courroux (<20 %) → Frappe du croisé → Jugement → Tempête divine → Consécration → Exorcisme → Courroux sacré (morts-vivants/démons). AoE : Consécration et Tempête divine en tête.
- **Protection** : rotation 969 — Bouclier sacré entretenu → Marteau du vertueux → Bouclier du vertueux → Consécration → Jugement
- **Sacré** : Guide de lumière + Bouclier sacré entretenus → Horion sacré / Eclair lumineux / Lumière sacrée selon la gravité

### Chevalier de la mort
Maladies entretenues (Fièvre de givre + Peste de sang) pour les trois arbres, [AoE : Mort et décomposition, Pestilence, Bouillonnement de sang], puis :
- **Sang** : Frappe de mort → Frappe du cœur/de sang → Frappe runique / Voile mortel
- **Givre** : Rafale hurlante (proc Brouillard givrant) → Oblitérer → Frappe de sang → Frappe de givre
- **Impie** : Frappe du fléau → Frappe de sang → Voile mortel

### Chasseur
Marque du chasseur, Tir automatique et familier engagés automatiquement ; Attaque du raptor si l'ennemi est au contact. Commandement de tuer → Tir de tuerie (<20 %) → [AoE : Volée, Flèches multiples] → Morsure de serpent → tir de spé (**BM** : —, **Précision** : Tir de la chimère puis Visée, **Survie** : Tir explosif puis Flèche noire) → Tir des arcanes → Tir assuré

### Voleur
Estocade entretenue dès 1 point de combo ; [AoE : Eventail de couteaux] ; puis :
- **Assassinat** : Soif de sang (avec saignement) → Envenimer (4-5 pts) → Mutilation
- **Combat** : Rupture (4-5 pts) → Eviscération (5 pts) → Attaque sournoise
- **Finesse** : Rupture (4-5 pts) → Eviscération (5 pts) → Hémorragie

### Prêtre
- **Ombre** : Forme d'ombre → [AoE : Fouet mental] → Toucher vampirique → Peste dévorante → Mot de l'ombre : Douleur → Attaque mentale → Mot de l'ombre : Mort → Fouet mental
- **Discipline** : Mot de pouvoir : Bouclier (hors Âme affaiblie) → Pénitence → Prière de guérison → Rénovation → Soins rapides
- **Sacré** : Cercle de soins / Prière de guérison / Rénovation / Soins rapides / Soins supérieurs selon la gravité

### Chaman
- **Elémentaire** : Bouclier d'eau → Horion de flammes → Salve de lave → [AoE : Chaîne d'éclairs] → Eclair
- **Amélioration** : Bouclier de foudre → Eclair/Chaîne d'éclairs (5 charges d'Arme du Maelström) → Frappe-tempête → Horion de flammes → Horion de terre → Fouet de lave
- **Restauration** : Bouclier d'eau + Bouclier de terre → Vague déferlante / Vague de soins inférieure / Salve de guérison / Vague de soins selon la gravité

### Mage
- **Arcanes** : [AoE : Explosion des arcanes, Blizzard] → Missiles des arcanes (proc Barrage ou 4 charges de Déflagration) → Déflagration des arcanes
- **Feu** : Pyroblast (proc Chaleur continue) → [AoE : Choc de flammes] → Bombe vivante → Boule de feu
- **Givre** : Elémentaire d'eau → [AoE : Blizzard] → Sarcophage de glace / Javelot de glace (proc Doigts de givre) → Sort de givre et de feu (proc « Boule de feu ! ») → Eclair de givre

### Démoniste
Connexion automatique si mana < 20 % ; familier engagé ; [AoE : Graine de corruption] ; puis :
- **Affliction** : Hantise → Corruption → Affliction instable → Malédiction d'agonie → Drain d'âme (<25 %) → Trait de l'ombre
- **Démonologie** : Malédiction d'apocalypse → Corruption → Immolation → Feu de l'âme (proc Décimation) → Incinérer (proc Cœur de la fournaise) → Trait de l'ombre
- **Destruction** : Immolation → Conflagration → Trait du chaos → Malédiction d'apocalypse → Incinérer

### Druide
- **Equilibre** : Forme de sélénien → Lucioles → Essaim d'insectes → Feu lunaire → [AoE : Pluie d'étoiles, Ouragan] → Feu stellaire (Eclipse lunaire) / Colère
- **Farouche** : suit la forme actuelle. **Ours** (tank) : Mutiler → Lucioles (farouche) → Lacérer x5 → [AoE : Balayage] → Mutilation. **Félin** : Fureur du tigre (énergie basse) → Rugissement sauvage → Déchirure (5 pts) → Griffure → Mutiler → Lambeau (repli Mutiler/Griffe)
- **Restauration** : Récupération / Croissance sauvage / Rétablissement / Prompte guérison / Nourrir selon la gravité

## Installation

1. Copier le dossier `mod-rotation` dans `modules/` des sources du serveur (à
   côté de `mod-playerbots`).
2. Relancer CMake puis recompiler.
3. (Optionnel) Personnaliser `configs/modules/mod_rotation.conf` ; rechargeable
   en jeu avec `.reload config`.

Aucun SQL n'est nécessaire.

## Structure du code

- [src/Rotation.h](src/Rotation.h) / [src/Rotation.cpp](src/Rotation.cpp) —
  configuration, interception du sort déclencheur (`OnPlayerSpellCast`, différé
  d'un tick), helpers (`TryCastRank`, `KeepAuraOn`, rang maximum connu…) et
  aiguillage classe/spécialisation (`GetMostPointsTalentTree`).
- `src/Rotation<Classe>.cpp` — un fichier par classe, une fonction par
  spécialisation : c'est là qu'on ajuste les priorités.
- `src/mod_rotation_loader.cpp` — point d'entrée du système de modules.

## Limites connues (v2)

- Les totems (Chaman), le maintien des buffs longs (Robustesse, Paroles de
  pouvoir…) et les gros cooldowns offensifs (Récklessness, Métamorphose,
  Ailes vengeresses…) ne sont pas automatisés — c'est voulu, pour laisser ces
  décisions au joueur.
- Lambeau (Voleur : Mutilation ; Druide : Lambeau) échoue silencieusement si
  le placement (dos de la cible, deux dagues…) n'est pas respecté ; un repli
  est prévu à chaque fois.
- Les sorts au sol (Blizzard, Volée, Mort et décomposition…) sont centrés sur
  la cible.

## Licence

[GNU GPL v2](LICENSE) — la même licence qu'AzerothCore.
