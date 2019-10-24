#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// The preselected order (m) is set to 4.
#define defOrd 4

// The smallest order is inevitably 3. The largest order is randomly selected.
#define minOrd 3
#define maxOrd 30

#define buffSz 256

//	TYPES

/* Type symbolizing the key's records:
 * This record would take the data (that is kept in a database),
 * a file (that is kept in an OS) or another data.
 */
typedef struct rec {
	int val;
} rec;


/* Type symbolizing a B+ tree's node:
 * It works for both the internal node and the leaf.
 * The array of the keys and array of equivalent pointers.
 * nKys is used to store the count of valid keys.
 * pNde is the parent node.
 */
typedef struct nde {
	void ** ptrs;
	int * kys;
	struct nde * pNde;
	bool isLeaf;
	int nKys;
} nde;


//	GLOBALS

/* Ord decides the largest and smallest number of node's 
 * items (keys and pointers).
 * The pre-set value is started by this global variable.
*/
int ord = defOrd;


/* The user controls when to switch off and on
 * the printing of hexadecimal characters beside their
 * respective key.
*/
bool ptrAddOut = false;


//	FUNCTIONS.

// 	Output and functionalities.

void message1(void);
void message2(void);
void get(nde * const rtNde, int ky, bool ptrAddr); 
void rng(nde * const rtNde, int range1, int range2, bool ptrAddr); 
int fndRng(nde * const rtNde, int kStrt, int keyEnd, bool ptrAddr,
		int rKys[], void * rPtrs[]); 
nde * fndLf(nde * const rtNde, int ky, bool ptrAddr);
rec * fnd(nde * rtNde, int ky, bool ptrAddr, nde ** lfOut);
int cut(int lg);

// Insert Operation.

rec * mRec(int val);
nde * mNde(void);
nde * mLf(void);
int getLIdx(nde * pNde, nde * lNde);
nde * putToLf(nde * lf, int ky, rec * ptr);
nde * putToLfWthSplit(nde * rtNde, nde * lf, int ky, rec * ptr);
nde * putToNde(nde * rtNde, nde * pNde, int lIdx, int ky, nde * rNde);
nde * putToNdeWithSplit(nde * rtNde, nde * pNde, int lIdx, int ky, nde * rNde);
nde * rtnIntoPnode(nde * rtNde, nde * lNde, int ky, nde * rNde);
nde * rtnIntoNRt(nde * lNde, int ky, nde * rNde);
nde * strtNTree(int ky, rec * ptr);
nde * put(nde * rtNde, int ky, int val);



//	DEFINING FUNCTION.

// OUTPUT AND UTILITIES

/* First message to the user.
 */
void message1(void) {
	printf("Hi there! To get started use any of the operations below :\n"
	"\tp <key>  -- put <key> (integer). Make it the key.\n"
	"\tp <key> <value> -- put the value <value> (integer) into the key <key> (integer).\n"
	"\tg <key>  -- get the value store with the key <key>.\n"
	"\tr <key_range_start_inclusive> <key_range_end_exclusive> -- Output location, keys and values stored in "
			"[<key1>, <key2>\n"
	"\ta -- Turn on or off the tree's and leaves' pointer addresses.\n"
	"\tq -- Close the application.\n"
	"\t? -- Output this menu\n");
}


/* Brief usage note.
 */
void message2(void) {
	printf("Usage: ./bpt [<ord>]\n");
	printf("\twhere %d <= ord <= %d .\n", minOrd, maxOrd);
}


/* Looks up a record after provided a key and 
 * outputs out a message depending on the results.
*/
void get(nde * const rtNde, int ky, bool ptrAddr) {
    nde * lf = NULL;
	rec * r = fnd(rtNde, ky, ptrAddr, NULL);
	if (r == NULL)
		printf("There was no record identified under the selected key key %d.\n", ky);
	else 
		printf("Location: %p; Key: %d; Value: %d.\n",
				r, ky, r->val);
}


/* Looks up and outputs the keys, values and pointers that is
 * between a range of keys (from keyStrt to keyEnd.)
*/
void rng(nde * const rtNde, int kStrt, int keyEnd,
		bool ptrAddr) {
	int i;
	int arrSiz = keyEnd - kStrt + 1;
	int rKys[arrSiz];
	void * rPtrs[arrSiz];
	int nFnd = fndRng(rtNde, kStrt, keyEnd, ptrAddr,
			rKys, rPtrs);
	if (!nFnd)
		printf("The range could not be identified.\n");
	else {
		for (i = 0; i < nFnd; i++)
			printf("Location: %p; Key: %d; Value: %d\n",
					rPtrs[i],
					rKys[i],
					((rec *)
					 rPtrs[i])->val);
	}
}
 

/* If keys and their respective pair are found in the given 
 * range, store them in rKys and rPtrs arrays and end my outputing
 * the number of entries available 
*/
int fndRng(nde * const rtNde, int kStrt, int keyEnd, bool ptrAddr,
		int rKys[], void * rPtrs[]) {
	int i, nFnd;
	nFnd = 0;
	nde * n = fndLf(rtNde, kStrt, ptrAddr);
	if (n == NULL) return 0;
	for (i = 0; i < n->nKys && n->kys[i] < kStrt; i++) ;
	if (i == n->nKys) return 0;
	while (n != NULL) {
		for (; i < n->nKys && n->kys[i] <= keyEnd; i++) {
			rKys[nFnd] = n->kys[i];
			rPtrs[nFnd] = n->ptrs[i];
			nFnd++;
		}
		n = n->ptrs[ord - 1];
		i = 0;
	}
	return nFnd;
}


/* Outlines the route between a root node and a leaf, 
 * looking by key. If address about the pointer is turned
 * on, it shows information about the route.
*/
nde * fndLf(nde * const rtNde, int ky, bool ptrAddr) {
	if (rtNde == NULL) {
		if (ptrAddr) 
			printf("Vacant tree.\n");
		return rtNde;
	}
	int i = 0;
	nde * c = rtNde;
	while (!c->isLeaf) {
		if (ptrAddr) {
			printf("[");
			for (i = 0; i < c->nKys - 1; i++)
				printf("%d ", c->kys[i]);
			printf("%d] ", c->kys[i]);
		}
		i = 0;
		while (i < c->nKys) {
			if (ky >= c->kys[i]) i++;
			else break;
		}
		if (ptrAddr)
			printf("%d ->\n", i);
		c = (nde *)c->ptrs[i];
	}
	if (ptrAddr) {
		printf("lf [");
		for (i = 0; i < c->nKys - 1; i++)
			printf("%d ", c->kys[i]);
		printf("%d] ->\n", c->kys[i]);
	}
	return c;
}


/* Looks for and returns the key's
 * record.
*/
rec * fnd(nde * rtNde, int ky, bool ptrAddr, nde ** lfOut) {
    if (rtNde == NULL) {
        if (lfOut != NULL) {
            *lfOut = NULL;
        }
        return NULL;
    }

	int i = 0;
    nde * lf = NULL;

	lf = fndLf(rtNde, ky, ptrAddr);

	/* If the root node is not "NULL", then there 
	 * must be a value in the leaf though it
	 * fails to have the wanted key.
	 * The leaf keeps the keys range which would
	 * have the wanted key
	*/

	for (i = 0; i < lf->nKys; i++)
		if (lf->kys[i] == ky) break;
    if (lfOut != NULL) {
        *lfOut = lf;
    }
	if (i == lf->nKys)
		return NULL;
	else
		return (rec *)lf->ptrs[i];
}

/* Looks for the best space to
 * divide a node which is getting
 * too huge into two.
*/
int cut(int lg) {
	if (lg % 2 == 0)
		return lg/2;
	else
		return lg/2 + 1;
}


// INSERTION OPERATION

/* Builds a fresh record to keep
 * the key's value.
*/
rec * mRec(int val) {
	rec * nRec = (rec *)malloc(sizeof(rec));
	if (nRec == NULL) {
		perror("record formation.");
		exit(EXIT_FAILURE);
	}
	else {
		nRec->val = val;
	}
	return nRec;
}


/* Builds a fresh node that can be used as a
 * leaf or an internal node
*/
nde * mNde(void) {
	nde * nNde;
	nNde = malloc(sizeof(nde));
	if (nNde == NULL) {
		perror("node formation.");
		exit(EXIT_FAILURE);
	}
	nNde->kys = malloc((ord - 1) * sizeof(int));
	if (nNde->kys == NULL) {
		perror("Fresh node keys group.");
		exit(EXIT_FAILURE);
	}
	nNde->ptrs = malloc(ord * sizeof(void *));
	if (nNde->ptrs == NULL) {
		perror("Fresh node pointers group.");
		exit(EXIT_FAILURE);
	}
	nNde->isLeaf = false;
	nNde->nKys = 0;
	nNde->pNde = NULL;
	return nNde;
}


/* Builds a fresh leaf through building
 * a node and adjusting it as needed.
*/
nde * mLf(void) {
	nde * lf = mNde();
	lf->isLeaf = true;
	return lf;
}


/* Auxiliary function for rtnIntoPnode
 * to locate the index of the parent node's
 * pointer to the node at the left of the key
 * that is being added.
*/
int getLIdx(nde * pNde, nde * lNde) {

	int lIdx = 0;
	while (lIdx <= pNde->nKys && 
			pNde->ptrs[lIdx] != lNde)
		lIdx++;
	return lIdx;
}


/* Adds a fresh pointer to a record and matching
 * key to a leaf.
 * Outputs the changed leaf.
*/
nde * putToLf(nde * lf, int ky, rec * ptr) {

	int i, insPt;

	insPt = 0;
	while (insPt < lf->nKys && lf->kys[insPt] < ky)
		insPt++;

	for (i = lf->nKys; i > insPt; i--) {
		lf->kys[i] = lf->kys[i - 1];
		lf->ptrs[i] = lf->ptrs[i - 1];
	}
	lf->kys[insPt] = ky;
	lf->ptrs[insPt] = ptr;
	lf->nKys++;
	return lf;
}


/* Adds a fresh key and pointer to a
 * fresh record into a leaf for it to surpass
 * the B+ tree's order, hence making the leaf
 * to be divided into two halfs.
*/
nde * putToLfWthSplit(nde * rtNde, nde * lf, int ky, rec * ptr) {

	nde * nLf;
	int * tKys;
	void ** tPtrs;
	int insIdx, split, nKy, i, j;

	nLf = mLf();

	tKys = malloc(ord * sizeof(int));
	if (tKys == NULL) {
		perror("Substitute keys group.");
		exit(EXIT_FAILURE);
	}

	tPtrs = malloc(ord * sizeof(void *));
	if (tPtrs == NULL) {
		perror("Substitute pointers group.");
		exit(EXIT_FAILURE);
	}

	insIdx = 0;
	while (insIdx < ord - 1 && lf->kys[insIdx] < ky)
		insIdx++;

	for (i = 0, j = 0; i < lf->nKys; i++, j++) {
		if (j == insIdx) j++;
		tKys[j] = lf->kys[i];
		tPtrs[j] = lf->ptrs[i];
	}

	tKys[insIdx] = ky;
	tPtrs[insIdx] = ptr;

	lf->nKys = 0;

	split = cut(ord - 1);

	for (i = 0; i < split; i++) {
		lf->ptrs[i] = tPtrs[i];
		lf->kys[i] = tKys[i];
		lf->nKys++;
	}

	for (i = split, j = 0; i < ord; i++, j++) {
		nLf->ptrs[j] = tPtrs[i];
		nLf->kys[j] = tKys[i];
		nLf->nKys++;
	}

	free(tPtrs);
	free(tKys);

	nLf->ptrs[ord - 1] = lf->ptrs[ord - 1];
	lf->ptrs[ord - 1] = nLf;

	for (i = lf->nKys; i < ord - 1; i++)
		lf->ptrs[i] = NULL;
	for (i = nLf->nKys; i < ord - 1; i++)
		nLf->ptrs[i] = NULL;

	nLf->pNde = lf->pNde;
	nKy = nLf->kys[0];

	return rtnIntoPnode(rtNde, lf, nKy, nLf);
}


/* Adds a new key as well as pointer to
 * a node without breaking any of the
 * B+ tree characteristics
*/
nde * putToNde(nde * rtNde, nde * n, 
		int lIdx, int ky, nde * rNde) {
	int i;

	for (i = n->nKys; i > lIdx; i--) {
		n->ptrs[i + 1] = n->ptrs[i];
		n->kys[i] = n->kys[i - 1];
	}
	n->ptrs[lIdx + 1] = rNde;
	n->kys[lIdx] = ky;
	n->nKys++;
	return rtNde;
}


/* Adds a fresh key as well as pointer into
 * a node which makes the size of the node to be more
 * than the order and thus making the node
 * to be divided into two halfs.
*/
nde * putToNdeWithSplit(nde * rtNde, nde * oNde, int lIdx, 
		int ky, nde * rNde) {

	int i, j, split, kPrm;
	nde * nNde, * ch;
	int * tKys;
	nde ** tPtrs;


	/* 1. Build a substitute group of keys and pointers
	 * to keep all of it in order, even the fresh key and pointer
	 * added in their accurate locations.
	 * 2. Build a fresh node and duplicate one-half of the keys 
	 * together with pointers to the older node and the remaining
	 * half to the fresh one.
	*/

	tPtrs = malloc((ord + 1) * sizeof(nde *));
	if (tPtrs == NULL) {
		perror("Substitute pointers group for dividing nodes.");
		exit(EXIT_FAILURE);
	}
	tKys = malloc(ord * sizeof(int));
	if (tKys == NULL) {
		perror("Substitute keys group for dividing nodes.");
		exit(EXIT_FAILURE);
	}

	for (i = 0, j = 0; i < oNde->nKys + 1; i++, j++) {
		if (j == lIdx + 1) j++;
		tPtrs[j] = oNde->ptrs[i];
	}

	for (i = 0, j = 0; i < oNde->nKys; i++, j++) {
		if (j == lIdx) j++;
		tKys[j] = oNde->kys[i];
	}

	tPtrs[lIdx + 1] = rNde;
	tKys[lIdx] = ky;
	
	/* Build a fresh node and duplicate
	 * one-half of the keys and pointers
	 * into the older node and the remaining
	 * to the fresh one
	*/
	split = cut(ord);
	nNde = mNde();
	oNde->nKys = 0;
	for (i = 0; i < split - 1; i++) {
		oNde->ptrs[i] = tPtrs[i];
		oNde->kys[i] = tKys[i];
		oNde->nKys++;
	}
	oNde->ptrs[i] = tPtrs[i];
	kPrm = tKys[split - 1];
	for (++i, j = 0; i < ord; i++, j++) {
		nNde->ptrs[j] = tPtrs[i];
		nNde->kys[j] = tKys[i];
		nNde->nKys++;
	}
	nNde->ptrs[j] = tPtrs[i];
	free(tPtrs);
	free(tKys);
	nNde->pNde = oNde->pNde;
	for (i = 0; i <= nNde->nKys; i++) {
		ch = nNde->ptrs[i];
		ch->pNde = nNde;
	}

	/* Places a fresh key to the parent node of the
	 * two nodes that came as a result of the split.
	 * The older node is made the left node whilst the
	 * fresh node is made the right node.
	*/
	return rtnIntoPnode(rtNde, oNde, kPrm, nNde);
}


/* Place a fresh node (either a leaf or an internal node)
 * inside the B+ tree.
 * Outputs rhe right node of tree after the operation insertion
 */
nde * rtnIntoPnode(nde * rtNde, nde * lNde, int ky, nde * rNde) {

	int lIdx;
	nde * pNde;

	pNde = lNde->pNde;

	/* Scenario: fresh right node */

	if (pNde == NULL)
		return rtnIntoNRt(lNde, ky, rNde);

	/* Scenario: leaf or node. */

	/* Look for the parent node's pointer at 
	 * the left node
	 */
	lIdx = getLIdx(pNde, lNde);


	/* Basic scenario: the fresh key enters inside the node. 
	 */

	if (pNde->nKys < ord - 1)
		return putToNde(rtNde, pNde, lIdx, ky, rNde);

	/* Complex scenario:  divide node to maintain 
	 * B+ tree characteristics.
	 */

	return putToNdeWithSplit(rtNde, pNde, lIdx, ky, rNde);
}


/* Build a fresh node at the right to allow for
 * two subtrees and place the correct key inside
 * the fresh right node
*/
nde * rtnIntoNRt(nde * lNde, int ky, nde * rNde) {

	nde * rtNde = mNde();
	rtNde->kys[0] = ky;
	rtNde->ptrs[0] = lNde;
	rtNde->ptrs[1] = rNde;
	rtNde->nKys++;
	rtNde->pNde = NULL;
	lNde->pNde = rtNde;
	rNde->pNde = rtNde;
	return rtNde;
}


/* Initial insertion function:
 * build a fresh tree.
*/
nde * strtNTree(int ky, rec * ptr) {

	nde * rtNde = mLf();
	rtNde->kys[0] = ky;
	rtNde->ptrs[0] = ptr;
	rtNde->ptrs[ord - 1] = NULL;
	rtNde->pNde = NULL;
	rtNde->nKys++;
	return rtNde;
}


/* Main insertion function
 * Add a key along with its value to B+ tree, and
 * making the tree change when needed to maintain
 * its characteristics.
*/
nde * put(nde * rtNde, int ky, int val) {

	rec * recPtr = NULL;
	nde * lf = NULL;


	/* The existing execution neglects 
	 * replicates
	*/

	recPtr = fnd(rtNde, ky, false, NULL);
    if (recPtr != NULL) {

		/* Replace the new value with the old value if the
		 * key already exists and output the tree.
		*/

        recPtr->val = val;
        return rtNde;
    }


	/* Build a fresh record for the
	 * value
	*/
	recPtr = mRec(val);


	/* Scenario: there is no current tree.
	 * Build a fresh tree
	*/ 

	if (rtNde == NULL) 
		return strtNTree(ky, recPtr);


	/* Scenario: there is a current tree.
	 * (Remaining function body.)
	*/ 

	lf = fndLf(rtNde, ky, false);


	/* Scenario: the leaf has space for both
	 * key and record pointer
	*/

	if (lf->nKys < ord - 1) {
		lf = putToLf(lf, ky, recPtr);
		return rtNde;
	}


	/* Scenario:  leaf has to divide.
	 */

	return putToLfWthSplit(rtNde, lf, ky, recPtr);
}


// MAIN

int main(int argc, char ** argv) {

	char * inF;
	FILE * fp;
	nde * rtNde;
	int iKy, iKy2;
	char ins;

	rtNde = NULL;
	ptrAddOut = false;

	if (argc > 1) {
		ord = atoi(argv[1]);
		if (ord < minOrd || ord > maxOrd) {
			fprintf(stderr, "Incorrect order: %d .\n\n", ord);
			message2();
			exit(EXIT_FAILURE);
		}
	}

    if (argc < 3) {
        message1();
    }

	if (argc > 2) {
		inF = argv[2];
		fp = fopen(inF, "r");
		if (fp == NULL) {
			perror("The input file could not be accessed.");
			exit(EXIT_FAILURE);
		}
		while (!feof(fp)) {
			fscanf(fp, "%d\n", &iKy);
			rtNde = put(rtNde, iKy, iKy);
		}
		fclose(fp);
        return EXIT_SUCCESS;
	}

	printf("> ");
    char buffer[buffSz];
    int count = 0;
    bool lCon = false;
	while (scanf("%c", &ins) != EOF) {
        lCon = false;
		switch (ins) {
		case 'p':
            fgets(buffer, buffSz, stdin);
            lCon = true;
            count = sscanf(buffer, "%d %d", &iKy, &iKy2);
            if (count == 1)
              iKy2 = iKy;
			rtNde = put(rtNde, iKy, iKy2);
			break;
		case 'g':
		case 'i':
			scanf("%d", &iKy);
			get(rtNde, iKy, ins == 'p');
			break;
		case 'r':
			scanf("%d %d", &iKy, &iKy2);
			if (iKy > iKy2) {
				int tmp = iKy2;
				iKy2 = iKy;
				iKy = tmp;
			}
			rng(rtNde, iKy, iKy2, ins == 'p');
			break;
		case 'q':
			while (getchar() != (int)'\n');
			return EXIT_SUCCESS;
			break;
		case 'a':
			ptrAddOut = !ptrAddOut;
			break;
		default:
			message1();
			break;
		}
        if (!lCon)
           while (getchar() != (int)'\n');
		printf("> ");
	}
	printf("\n");

	return EXIT_SUCCESS;
}