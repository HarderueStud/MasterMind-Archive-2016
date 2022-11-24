/*//==========================================================================================================================================================
//
//==========================================================================================================================================================
															CLEMENT **** . GROUPE B --- Archive 2016/2017
																MASTERMIND

FICHE TECHNIQUE :

	- Obtenir un programme robuste, le plus interactif possible avec le joueur.
		-> Choix pour le joueur de tester telle couleur � tel endroit.
			-> 2 tableaux � 2 dimensions pour tester l'�galit�
		-> Tout en restant avec des tableaux statiques ....
		-> "Combinaison magique" totalement al�atoire en utilisant la librairie <random>.
			-> Un hasard g�r� avec le temps grace � une graine time (seed).
			-> Distribution/g�n�ratrice != pseudo-random.
		-> Param�trage de partie (Pseudo, nombre de ligne pour le plateau, mode triche...)
			-> Les changements de param�tre pour les parties sont gard�s et restent par d�faut ten que le programme ne red�marre pas
				-> Passage par r�f�rence plut�t que par adresse car notation des pointeurs * assez lourde pour le m�me r�sultat.
		-> Sauvegarde automatique du pseudo, de la solution et de si le joueur � gagner ou non avec le nombre d'essais qu'il aura eus et combien il en aura eu
			besoin s'il a gagn�.
				-> <Stream> (file stream)
		-> G�rer les couleurs sans faire de crises d'�pilepsie, sans Sfml/Sdl/Qt.
			-> <widows.h>
		-> Code couleur dans le jeu (1 -> JAUNE; 2 -> BLEU; 3 -> ROUGE; 4 -> VERT; 5 -> BLANC; 6 -> NOIR)

	Programme fait sur Visual Studio 17, capture d'�cran sur picPick.

	SI vous rencontrez un probl�me de compilation sur CodeBlock c'est probablement du aux fonctions de C++11/14 MAIS,
		-> La solution est simple, sur CodeBlock faire : 
			1. Settings -> Compiler -> Compiler Flags -> cocher:"Have g++ follow the C++14 ISO C++ language standard"
			2. cocher:"Have g++ follow the C++11 ISO C++ language standard"
			
//==========================================================================================================================================================
//
//==========================================================================================================================================================*/

//Input & output stream :
#include <iostream> 
//Manipulation des chaines de caract�res
#include <string>
//Cr�ation de nombres aleatoire :
//	-> pour c++11
#include <random>
//	-> sinon pour utiliser std::rand()%..
//#include <cstdlib> 
//"Seed"
#include <ctime>
//Manipulation fichiers :
#include <fstream> 
//Header file qui fournit les couleurs parmis d'autres choses
#include <Windows.h>
//
//#include <vector>

//Utilisation exclusive des �l�ments dont l'on a besoin dans l'espace du nom std
using std::cout;
using std::endl;
using std::cin;

//Utilisation d'alias pour gagner du temps sur la d�claration des variables redondantes
using unshort = unsigned short;
//		-> OU
//typedef unsigned short unshort;
//Using plus d'actualiser + compatible avec les templates.

// �num�ration : chaque couleur qui seront pr�sents dans le MASTERMIND prennent leur valeur ici (noir = 0; gris fonc� = 7; blanc = 15...
enum couleur {
	BLACK = 0, //Par default
	DARKGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	GRAY = FOREGROUND_INTENSITY,
	BLUE = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
	GREEN = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
	RED = FOREGROUND_INTENSITY | FOREGROUND_RED,
	YELLOW = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
	WHITE = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
};

//Une variable GLOBALE
const unshort c_nbrColonne{ 4 };

//==========================================================================================================================================================
//									D�claration des fonctions... Ou proc�dures
//==========================================================================================================================================================

//Hi�rarchiser avec des Tab pour savoir lesquelles travaillent avec lesquelles. La plupart vont prendre des r�f�rences pour param�tre (&)
// -> J'ai pr�f�r� �viter les passages par adresse � cause de l'�criture lourde que repr�sentent les * d�s pointeurs, m�me si au final le r�sultat aurait �t� le m�me.

//la fonction "m�re" du jeu qui appellera le menu
void f_Aff_Presentation();
//Le menu va proposer plusieurs choix au joueur : Jouer, afficher les r�gles, quitter ou...
void f_Menu();
//Une fois la fonction f_JEU() lancer, celle-ci va appeler une armada d'autres fonction pour faire tourner le jeu, configuration de la partie, sauvegarde etc..
void f_JEU();
//Param�trage de la partie
void f_parametrePartie(std::string& p_pseudoJoueur, unshort& p_nbrLigne, bool& p_cheat);
//Cr�ation de la "formule magique" que notre BOT Jojo va faire deviner au joueur
void f_CreationSolution(const unshort& p_nbrCouleur, const unshort& p_nbrColonne, unshort p_t_combiSecrete[][c_nbrColonne], bool& p_cheat);
//Un simple test pour v�rifier le remplissage de la solution -> pas pr�sent en jeu
void f_Aff_TestSolution(unshort p_t_combiSecrete[][c_nbrColonne]);
//La partie "gameplay" ou le joueur va jouer/� demander de l'aide etc...
void f_JoueurJoue(unshort& p_nbrLigne, const unshort& p_nbrColonne, unshort p_t_combiSecrete[][c_nbrColonne], bool& p_gagner, unshort& p_nbrEssai, bool& p_cheat);
//Appel�e une foi que la partie est finie, d�termine si une nouvelle partie doit �tre relanc�e (avec les m�mes param�tres que la pr�c�dente s'il decide de pas changer)
void f_rejouer(bool& p_rejouer);
//Sauvegarde de la partie dans un fichier texte que j'ai nomm� "sauvegarde", normalement le fichier sera cr�� automatiquement dans le r�pertoire de l'exe
void f_SauvegardePartie(const std::string& p_pseudoJoueur, const unshort p_t_combiSecrete[][c_nbrColonne], unshort& p_nbrEssai, unshort& p_nbrLigne, bool& p_cheat);
//Affichage des r�gles, que ce soit lorsque le joueur est sur le menu du programme ou lorsqu'il tapera "66" en pleine partie.
void f_Aff_Regle();
//A
void a();
//Quitter l'application, avec une v�rification pour savoir si le joueur est sur de vouloir �teindre.
void f_Quitter();

//Fonction qui affiche lorsque le joueur a gagner ou � perdu, si perdu va afficher la solution. (J'aurais pu mettre l'affichage de la solution directement
//dans la fonction f_JoueurJoue(), �a revient au m�me
void f_Aff_JoueurGagne(unshort& p_nbrEssai, unshort& p_nbrLigne);
void f_Aff_JoueurPerd(unshort p_t_combiSecrete[][c_nbrColonne]);

//==========================================================================================================================================================
//										MAIN
//==========================================================================================================================================================

int main()
{
	f_Aff_Presentation();
	return 0;
}

//==========================================================================================================================================================
//									Fonctions
//==========================================================================================================================================================
void f_Aff_Presentation()
{
	cout << "-------------------------------:::::::::::::::::::::::::::--------------------------------\n" << endl;
	cout << "	::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;
	cout << "	::::::::::::------------- BIENVENUE, DANS ZE MASTERMIND ------------::::::::::::" << endl;
	cout << "	::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::" << endl;
	cout << "\n-------------------------------:::::::::::::::::::::::::::--------------------------------" << endl;
	f_Menu();
}

void f_Menu()
{
	const unshort nbrOptionMenu{ 4 };
	//Affichage du menu
	cout << "\n	>MENU : " << endl;
	cout << "\n			>Jouer    <1>" << endl;
	cout << "\n			>Regles   <2>" << endl;
	cout << "\n			>Bonus	  <3>" << endl;
	cout << "\n			>QUITTER  <4>" << endl;

	{
		unshort repMenu{ NULL };
		cout << "\n-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<" << endl;
		cout << "\nQue voulez-vous faire ? " << endl;
		cout << "> "; cin >> repMenu;

		while (repMenu < 1 || repMenu > nbrOptionMenu)
		{
			cout << "\n /!\\ Erreur : La reponse est incorrect ! veuillez verifier que votre choix est parmis les services proposer." << endl;
			cout << "\n> Reessayer : "; cin >> repMenu;
		}
		switch (repMenu)
		{
		case 1: f_JEU(); break;
		case 2: f_Aff_Regle(); break;
		case 3: a(); break;
		case 4: f_Quitter(); break;
		}
	}
}

void f_JEU()
{
	const unshort c_nbrCouleur{ 6 };
	unshort c_nbrLigne{ 10 }, nbrEssai{ 0 };
	std::string p_pseudoJoueur{ "Tiplouf" };
	unshort t_combiSecrete[2][c_nbrColonne];
	bool gagner{ false }, cheat{ false }, rejouer{ true }; 

	cout << "\n///////////////////////////////////////////////////////////////////////////" << endl;
	cout << "\n>Yes ! Quelqun veut ce mesurer a moi ! L'imbattable JoJo !! " << endl;

	while (rejouer)
	{
		//OK
		f_parametrePartie(p_pseudoJoueur, c_nbrLigne, cheat);
		//OK
		f_CreationSolution(c_nbrCouleur, c_nbrColonne, t_combiSecrete, cheat);
		//OK
		//f_Aff_TestSolution(t_combiSecrete);
		//OK
		f_JoueurJoue(c_nbrLigne, c_nbrColonne, t_combiSecrete, gagner, nbrEssai, cheat);
		//OK
		f_SauvegardePartie(p_pseudoJoueur, t_combiSecrete, nbrEssai, c_nbrLigne, cheat);
		//OK
		f_rejouer(rejouer);
	}
}

void f_parametrePartie(std::string& p_pseudoJoueur, unshort& p_nbrLigne, bool& p_cheat)
{
	cout << "\n>OUPS, J'oubliais, il faut revoir les parametre de la partie avant de pouvoir lancer celle-ci... " << endl;
	cout << "\nLes parametres par default : " << endl;
	cout << "		-Pseudo : " << p_pseudoJoueur << endl;
	cout << "		-Difficulter : ";
	if (p_nbrLigne >= 2 && p_nbrLigne <= 4)
		cout << "Difficile ";
	else if (p_nbrLigne > 4 && p_nbrLigne <= 10)
		cout << "Normal ";
	else
		cout << "Facile ";
	cout << "(" << p_nbrLigne << " essais)" << endl;
	cout << "		-Nombre de trous par ligne : 4* ";
	cout << "\n		-Nombre de couleur : 6* ";
	cout << "\n		-Cheat : ";
	if (p_cheat)
		cout << " ACTIV� /!\\ NO FUN ";
	else
		cout << " DESACTIV�";
	cout << "\n* : Non modifiable." << endl;
	{
		char repPara{ NULL };
		cout << "\n///////////////////////////////////////////////////////////////////////////" << endl;
		cout << ">Vous avez la possibiliter de changer la difficulter (en modifiant directement le nombre de ligne) et votre pseudo.";
		cout << "\n>Voulez-vous le faire ? (o/n) : "; cin >> repPara;

		while (repPara != 'o' && repPara != 'O' && repPara != 'n' && repPara != 'N')
		{
			cout << "\n/!\\ Erreur : Reponse inccorrect... soit O pour oui soit N pour non. " << endl;
			cout << "\n> Reessayer : "; cin >> repPara;
		}
		if (repPara == 'n' || repPara == 'N')
		{
			cout << "\n>Tres bien ! alors c'est partit !! " << endl;
			cout << "\n///////////////////////////////////////////////////////////////////////////" << endl;
		}
		else
		{
			cout << endl;
			cout << "\nHmm... Vous etes bien temeraire... a moin que ce soit juste pour changer de pseudo ? ou alors pour baisser la difficulter ? OU... POUR CHEAT ?? HAHAHA auriez-vous peur du GRAND JoJo ?? HAHA..." << endl;
			//Debut des changement de parametres : Pseudo -> nbr essai = nbr ligne = difficulter -> activer la triche...
			cout << "\nReglons dans un premier temps votre pseudo : "; cin >> p_pseudoJoueur;
			cout << "\nTres bien " << p_pseudoJoueur << ", combien d'essais choisirez vous ? "; cin >> p_nbrLigne;

			while (p_nbrLigne < 3 || p_nbrLigne > 15)
			{
				cout << "Hmm... laissez moi reflechir... NON, on met plus de 2 essais et pas plus de 15 essais sinon le GRAND JoJo va s'enerver ! " << endl;
				cout << ">Reessayer : "; cin >> p_nbrLigne;
			}
			cout << "\n>La difficulter sera donc en ";
			if (p_nbrLigne > 0 && p_nbrLigne < 5)
				cout << "Difficile ";
			else if (p_nbrLigne >= 5 && p_nbrLigne <= 10)
				cout << "Normal ";
			else
				cout << "Facile ";
			cout << " avec " << p_nbrLigne << " essais ! Bonne chance " << p_pseudoJoueur << " ! " << endl;
			{
				char repCheat{ NULL };
				cout << "\nActiver le cheat ? (o/n) : "; cin >> repCheat;
				//Preferable si on veut verifier que le programme marche et que la solution marche bien avec les reponses du joueur
				while (repCheat != 'o' && repCheat != 'O' && repCheat != 'n' && repCheat != 'N')
				{
					cout << "\n/!\\ Erreur : Reponse inccorrect... ET en plus c'est PAS bien de tricher... " << endl;
					cout << "\n> Reessayer : "; cin >> repCheat;
				}
				if (repCheat == 'o' || repCheat == 'O')
				{
					p_cheat = true;
					cout << "\n>>>Cheat activer." << endl;
				}
				else
				{
					p_cheat = false;
					cout << "\n>>>Cheat desactiver." << endl;
				}
			}
			cout << "\n///////////////////////////////////////////////////////////////////////////" << endl;
		}
	}
}

//C'est ici, que toute la magie opere
void f_CreationSolution(const unshort& p_nbrCouleur, const unshort& p_nbrColonne, unshort p_t_combiSecrete[][c_nbrColonne], bool& p_cheat)
{
	//C'est ici qu'on va ce servir de la librairie <random>
	std::default_random_engine random(time(NULL)); //Generateur -> engine + une graine (seed) : temps pour avoir un vrai random != pseudo-random
	std::uniform_int_distribution<unshort>distributionC{ 1, p_nbrCouleur }; //Distribution classique -> uniforme entre 1 et la constante nombre de couleur(6) pour les couleurs

	//std::srand(std::time(NULL));

	for (unshort i{ 0 }; i < p_nbrColonne; i++)
	{
		//Determination de la formule magique pour gagner !
		p_t_combiSecrete[0][i] = i;
		p_t_combiSecrete[1][i] = distributionC(random);

		if (p_cheat)
			cout << "Position : " << p_t_combiSecrete[0][i] + 1 << " Couleur : " << p_t_combiSecrete[1][i] << endl;
		else
			cout << "\nPreparation de la formule magique... ... " << endl;
	}
}

//Les tests pour verifier le remplissage -> plus tres utile
void f_Aff_TestSolution(unshort p_t_combiSecrete[][c_nbrColonne])
{
	//TEST Position --> couleur -> OK
	cout << endl << endl;
	cout << p_t_combiSecrete[0][0] << " p->c " << p_t_combiSecrete[1][0] << endl; //{0,0}
	cout << p_t_combiSecrete[0][1] << " p->c " << p_t_combiSecrete[1][1] << endl; //{0,1}
	cout << p_t_combiSecrete[0][2] << " p->c " << p_t_combiSecrete[1][2] << endl; //{0,2}
	cout << endl << endl;
}

//Le "Gameplay", c'est ici que le joueur va pouvoir faire ces preuves contre le grand JoJo
void f_JoueurJoue(unshort& p_nbrLigne, const unshort& p_nbrColonne, unshort p_t_combiSecrete[][c_nbrColonne], bool& p_gagner, unshort& p_nbrEssai, bool& p_cheat)
{
	//Variable -> certaine doivent imperativement etre initialiser, d'autre non.
	unshort pionNoir{ 0 }, pionBlanc{ 0 }, repCouleur{ 0 }, repPosition{ 0 };
	unshort t_combiTest[2][c_nbrColonne];

	//Non.. Non.. j'ai dis pas de tableau dynamique...
	//std::vector<std::vector<unshort>>t_combiTest; //tableau de tableau dy

	//Les.. COULEURS :
	HANDLE h_Console;
	h_Console = GetStdHandle(STD_OUTPUT_HANDLE);
	couleur col{ WHITE };  //on met par default la couleur sur blanc -> 15

	cout << "\n\n Si vous avez besoin d'un rappel des regles durant la partie je vous invite a rentrer 66 dans position pour vous rediriger vers les regles." << endl;
	bool help = false; //position -> 66
	for (unshort i{ 0 }; i < p_nbrLigne; i++)
	{
		for (unshort t{ 0 }; t < 2; t++)
		{
			for (unshort z{ 0 }; z < c_nbrColonne; z++)
			{
				t_combiTest[t][z] = 0;
				// DEBUG -> OK valeur a 0
				// cout << t_combiTest[t][z] << " ";
			}
			cout << "\n";
		}
		cout << "\nLigne numero : " << i + 1 << endl; //Le +1 pour gerer l'affichage
		for (unshort j{ 0 }; j < p_nbrColonne; j++)
		{
			repCouleur = 0; repPosition = 0;
			cout << "Position : "; cin >> repPosition;
			if (repPosition == 66)
			{
				j = p_nbrColonne + 1;
				i = p_nbrLigne + 1;
				help = true;
			}
			else
			{
				while (repPosition < 1 || repPosition>4)
				{
					cout << "\nErreur : la position est entre 1 et 4 ! (ou 66 pour revoir les regles)" << endl;
					cout << "\nPosition : ";  cin >> repPosition;
				}
				while (t_combiTest[1][repPosition - 1] != 0)
				{
					cout << "\nErreur : la position a deja etait tester !" << endl;
					cout << "\nPosition : ";  cin >> repPosition;

					while (repPosition < 1 || repPosition>4)
					{
						cout << "\nErreur : la position est entre 1 et 4 ! (ou 66 pour revoir les regles)" << endl;
						cout << "\nPosition : ";  cin >> repPosition;
					}
				}
				repPosition--;
				cout << "Couleur : (1 -> JAUNE; 2 -> BLEU; 3 -> ROUGE; 4 -> VERT; 5 -> BLANC; 6 -> NOIR) : " << endl;
				cout << "		> "; cin >> repCouleur;
				//TEST-> OK
				while (repCouleur < 1 || repCouleur>6)
				{
					cout << "\nErreur : la couleur est entre 1 et 6 !" << endl;
					cout << "(1 -> JAUNE; 2 -> BLEU; 3 -> ROUGE; 4 -> VERT; 5 -> BLANC; 6 -> NOIR) ->" << endl;
					cout << "\n		>Couleur : ";  cin >> repCouleur;
				}

				//Remplissage
				t_combiTest[0][repPosition] = repPosition;
				t_combiTest[1][repPosition] = repCouleur;

				/*  Des tests pour verifier que les reponses du joueurs sont bien misent dans le tableau -> ca marche niquel
				cout << endl;
				for (unshort t { 0 }; t < 2; t++)
				{
					for (unshort z { 0 }; z < c_nbrColonne; z++)
					{
						cout << t_combiTest[t][z] << " ";
					}
					cout << "\n";
				}
				*/
			}
			if (repCouleur == p_t_combiSecrete[1][repPosition] && repPosition == p_t_combiSecrete[0][repPosition]) //
			{
				pionNoir++;
				//OK
				// cout << "NOIR -> OK couleur bonne endroit" << endl; 
			}
			else
			{
				for (unshort y{ 1 }; y <= p_nbrColonne; y++)
				{
					if (repCouleur == p_t_combiSecrete[1][y]) //
					{
						pionBlanc++;
						//OK
						//cout << "BLANC -> couleur mauvaise endroit" << endl;
					}
				}
			}

		}
		cout << "\t\t\t";
		for (unshort t{ 0 }; t < c_nbrColonne; t++)
		{
			if (t_combiTest[1][t] == 1)
				col = YELLOW;
			else if (t_combiTest[1][t] == 2)
				col = BLUE;
			else if (t_combiTest[1][t] == 3)
				col = RED;
			else if (t_combiTest[1][t] == 4)
				col = GREEN;
			else if (t_combiTest[1][t] == 5)
				col = WHITE;
			else if (t_combiTest[1][t] == 6)
				col = GRAY;
			else
				col = BLACK;

			SetConsoleTextAttribute(h_Console, col);
			cout << " O ";
			col = WHITE;
			SetConsoleTextAttribute(h_Console, col);
		}
		cout << endl;
		cout << "\n>Pion Noir ";
		col = GRAY;
		SetConsoleTextAttribute(h_Console, col);
		cout << " O ";
		col = WHITE;
		SetConsoleTextAttribute(h_Console, col);
		cout << ": " << pionNoir << endl;
		cout << "\n>Pion Blanc  O : " << pionBlanc << endl;

		if (pionNoir >= p_nbrColonne)
		{
			p_nbrEssai = i + 1;
			i = p_nbrLigne; 
			p_gagner = true;
		}
		else
		{
			pionNoir = 0;   
			pionBlanc = 0;
			p_gagner = false;
		}
	}
	if (p_cheat)
		cout << "\n	\t\t\t\t CHEAT : ACTIVER" << endl;

	if (p_gagner)
	{
		f_Aff_JoueurGagne(p_nbrEssai, p_nbrLigne);
	}
	else
	{
		f_Aff_JoueurPerd(p_t_combiSecrete);
		if (help)
			f_Aff_Regle();
		p_nbrEssai = p_nbrLigne + 1;
	}
}

//""
void f_rejouer(bool& p_rejouer)
{
	char repRejouer{ NULL };
	cout << "\n***************************************************************" << endl;
	cout << "\n***************************************************************" << endl;
	cout << "\n		>Voulez vous relancer une partie ?? (o/n) " << endl;
	cout << "\n***************************************************************" << endl;
	cout << "\n> ";
	cin >> repRejouer;
	while (repRejouer != 'o' && repRejouer != 'O' && repRejouer != 'n' && repRejouer != 'N')
	{
		cout << "\n/!\\ Erreur : J'ai pas compris, pouvez vous repetez ?? " << endl;
		cout << "\n> Reessayer : "; cin >> repRejouer;
	}
	if (repRejouer == 'o' || repRejouer == 'O')
	{
		p_rejouer = true;
		cout << "\nVous decidez de relancer une partie ! quel courage !!" << endl;
	}
	else
	{
		p_rejouer = false;
		cout << "\nC'est donc un retour vers le menu principal..." << endl;
		cout << "\n***************************************************************" << endl;
		f_Menu();
	}
	cout << "\n***************************************************************" << endl;
}

//Idem
void f_SauvegardePartie(const std::string& p_pseudoJoueur, const unshort p_t_combiSecrete[][c_nbrColonne], unshort& p_nbrEssai, unshort& p_nbrLigne, bool& p_cheat)
{
	std::string const nomFichier{ R"(sauvegarde.txt)" };
	std::ofstream flux_Sauvegarde_P{ nomFichier, std::ios::app }; //ios::app -> place le curseur directement a la fin du fichier si il existe deja
	if (flux_Sauvegarde_P)
	{
		//cout << "\nOuverture de sauvegarde.txt -> OK " << endl;
		flux_Sauvegarde_P << "\n-----------------------------------------------------" << endl;
		flux_Sauvegarde_P << "\n	MASTERMIND SAUVEGARDE : " << p_pseudoJoueur << endl;
		flux_Sauvegarde_P << "La combinaison secrete : \n";
		for (unshort i{ 0 }; i < c_nbrColonne; i++)
			flux_Sauvegarde_P << "\tPosition : " << p_t_combiSecrete[0][i] + 1 << " Couleur : " << p_t_combiSecrete[1][i] << endl;
		if (p_nbrEssai < p_nbrLigne + 1)
			flux_Sauvegarde_P << "Vous avez gagner la partie en " << p_nbrEssai << "/" << p_nbrLigne << " essais !" << endl;
		else
			flux_Sauvegarde_P << "Vous avez perdu la partie... Vous aviez " << p_nbrEssai - 1 << " Essais." << endl;
		if (p_cheat)
			flux_Sauvegarde_P << "CHEAT : ACTIVER" << endl;
		cout << "Sauvegarde effectuer sous format texte au nom de \"sauvegarde\"." << endl;
		flux_Sauvegarde_P.close();
		//cout << "Fermeture de sauvegarde.txt -> OK" << endl;
	}
	else
	{
		cout << "\n/!\\ Erreur : Ouverture de sauvegarde.txt -> FAILED " << endl;
	}
}

//L'affichage des regles
void f_Aff_Regle()
{
	cout << "\n->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->" << endl;
	cout << "-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<->->->->->->->->->->->->->->->" << endl;
	cout << "		  											 THE RULES		   										   " << endl;
	cout << "\n->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->" << endl;
	cout << "-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<->->->->->->->->->->->->->->->" << endl;
	cout << endl;
	cout << "\n> Je vais essayer d'etre le plus clair possible : " << endl;
	cout << "\n	-Un nombre de ligne changeant selon les parametre de la partie, ce qui correspond a la difficulter : " << endl;
	cout << "		>Entre 2 et 4 ligne > Difficile" << endl;
	cout << "		>Entre 5 et 10 ligne > Normal" << endl;
	cout << "		>Entre 10 et 15 ligne > Facile" << endl;
	cout << "\n	-Ce MASTERMIND ce jouera avec des boules de differentes couleurs (6 couleurs) : " << endl;
	cout << "		>Bleu, Jaune, Vert, Rouge, Blanc, Noir" << endl;
	cout << "	-Chaque trous de ligne correspond a un positionnement de 1 a 4 en partant de la gauche." << endl;
	cout << "\n -Le joueur devra entrer la position de la ligne avec la couleur a tester dans le but de retrouver la combinaison cree par JoJo." << endl;
	cout << "\n -Aider avec des indices donner apres chaque ligne tester : " << endl;
	cout << "		>Nombre de pion Noir  : Le positionnement de la boule ainsi que sa couleur sont correct." << endl;
	cout << "		>Nombre de pion Blanc  : Le positionnement de la boule est incorrect MAIS la couleur est presente, autre part." << endl;
	cout << "\n					!	BON JEU !  " << endl;
	cout << "\n->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->" << endl;
	cout << "-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<->->->->->->->->->->->->->->->" << endl;
	{
		char n{ NULL };
		cout << "\nAppuyer sur n'importe quel touche pour passer a la suite !! " << endl;
		cin >> n;
	}
	cout << endl;
	f_Menu();
}

//JoJo
void a()
{
	cout << "\n->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->" << endl;
	cout << "-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<->->->->->->->->->->->->->->->" << endl;
	cout << endl;
	cout << "\nYop c'est JoJo, non plus dans le jeu des allumettes, mais cette foi dans ZE MASTERMIND ! " << endl;
	cout << "      )-(        " << endl;
	cout << "     (O O)           Plusieurs passions dans la vie," << endl;
	cout << "      \\=/			    le.. ..  MASTERMIND ...      " << endl;
	cout << "     .-\"-.               et les animaux.           " << endl;
	cout << "    //\\ /\\\\      " << endl;
	cout << "  _// / \\ \\\\_              /)/)  " << endl;
	cout << " =./ {,-.} \\.=            (+.+)    " << endl;
	cout << "     || ||               (\")_(\")  " << endl;
	cout << "     || ||       " << endl;
	cout << "   __|| ||__     " << endl;
	cout << "  `---" "---'    " << endl;
	cout << "\nJ'ai hate de REJouer avec vous !! " << endl;
	cout << "\n->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->" << endl;
	cout << "-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<->->->->->->->->->->->->->->->" << endl;
	{
		char n{ NULL };
		cout << "\nAppuyer sur n'importe quel touche pour retourner au menu ! ";
		cin >> n;
	}
	f_Menu();
}

void f_Quitter()
{
	char repQuitter{ NULL };
	cout << "/!\\ Etes vous sur de vouloir quitter ?? VRAIMENT sur ? (o/n) /!\\ : ";
	cin >> repQuitter;
	while (repQuitter != 'o' && repQuitter != 'O' && repQuitter != 'n' && repQuitter != 'N')
	{
		cout << "\n/!\\ Erreur : Reponse... Pas comprendu... " << endl;
		cout << "\n> Reessayer : "; cin >> repQuitter;
	}
	if (repQuitter == 'n' || repQuitter == 'N')
	{
		f_Menu();
	}
	else
	{
		cout << "\n Ce n'est qu'un en revoir... a BIENTOOOT ! " << endl;
		cout << "                __/\\__               " << endl;
		cout << "               `==/\\==               " << endl;
		cout << "     ____________/__\\____________    " << endl;
		cout << "    /____________________________\\   " << endl;
		cout << "      __||__||__/.--.\\__||__||__     " << endl;
		cout << "     /__|___|___( >< )___|___|__\\    " << endl;
		cout << "               _/`--`\\_              " << endl;
		cout << "              (/------\\)             " << endl;

		char n{ NULL };
		cout << "Appuyer sur n'importe quel touche pour eteindre le programme ! " << endl;
		cin >> n;
	}
	cout << "\n->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->->" << endl;
	cout << "-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<->->->->->->->->->->->->->->->" << endl;
}

void f_Aff_JoueurGagne(unshort& p_nbrEssai, unshort& p_nbrLigne)
{
	cout << "\nVous avez Gagner !! Vous avez trouver la bonne combinaison ! " << endl;
	cout << "\t et en seulement ... " << p_nbrEssai << " sur " << p_nbrLigne << " essais !" << endl;
	cout << "_______Z " << endl;
	cout << "_______Z_Z" << endl;
	cout << "_______Z__ZZ" << endl;
	cout << "_______Z____ZZZZZZZZ" << endl;
	cout << "______ZZ___________ZZ" << endl;
	cout << "____ZZZ____________Z" << endl;
	cout << "__ZZ______________Z" << endl;
	cout << "ZZ_______________ZZZZZZZZZZZZ" << endl;
	cout << "ZZZ______________ZZZ_________ZZZZZ" << endl;
	cout << "___ZZZ____________ZZZZZZ__________ZZZZ" << endl;
	cout << "______ZZ___________Z____ZZ____________ZZZ" << endl;
	cout << "_______Z_____ZZZ____Z_____ZZZ____________ZZ" << endl;
	cout << "_______Z___ZZ___ZZZZZ________ZZ____________ZZ" << endl;
	cout << "_______Z__Z____________________ZZ____________ZZ" << endl;
	cout << "_______ZZZ_______________________ZZ___________ZZ" << endl;
}

void f_Aff_JoueurPerd(unshort p_t_combiSecrete[][c_nbrColonne])
{
	cout << "\n		Vous avez Perdu !! ";
	cout << "\n La combinaison magique etait : \n" << endl;

	{
		HANDLE h_Console;
		h_Console = GetStdHandle(STD_OUTPUT_HANDLE);
		couleur col{ WHITE };
		cout << "\t\t\t";
		for (unshort y{ 0 }; y < c_nbrColonne; y++)
		{
			if (p_t_combiSecrete[1][y] == 1)
				col = YELLOW;
			else if (p_t_combiSecrete[1][y] == 2)
				col = BLUE;
			else if (p_t_combiSecrete[1][y] == 3)
				col = RED;
			else if (p_t_combiSecrete[1][y] == 4)
				col = GREEN;
			else if (p_t_combiSecrete[1][y] == 5)
				col = WHITE;
			else if (p_t_combiSecrete[1][y] == 6)
				col = GRAY;
			else
				col = BLACK;

			SetConsoleTextAttribute(h_Console, col);
			cout << " O ";
			col = WHITE;
			SetConsoleTextAttribute(h_Console, col);
		}
		cout << endl;
	}
	//
	cout << "________________________��__���__��" << endl;
	cout << "_________________________�___�___�" << endl;
	cout << "______�_____________�____�___�___�" << endl;
	cout << "____�_�_____________�_�___�__�__�" << endl;
	cout << "___�___�___________�___�___�����" << endl;
	cout << "___�____�����������____�_____�" << endl;
	cout << "____�___�_�������_�___�______�" << endl;
	cout << "_____����__�����___���_______�" << endl;
	cout << "____���_____���______���_____�" << endl;
	cout << "___��__________________��____�" << endl;
	cout << "___��___���_____���____��____�" << endl;
	cout << "___��____���___���_____��____�" << endl;
	cout << "___��_�______________�_��____�" << endl;
	cout << "___��_�����______�����_��__���" << endl;
	cout << "___��___�_��������_�___�����_���" << endl;
	cout << "___��_____�_�__�_�_____��____�__�" << endl;
	cout << "____���____������____�����������" << endl;
	cout << "______���__________���_______�" << endl;
	cout << "________���__��__���_________�" << endl;
	cout << "__________��������___________�" << endl;
	cout << "____________�����____________�" << endl;
	cout << "_____________���_____________�" << endl;
	cout << "______________�______________�" << endl;
}

//==========================================================================================================================================================
//									
//==========================================================================================================================================================