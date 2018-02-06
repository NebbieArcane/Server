/*$Id: charlist.h,v 1.2 2002/02/13 12:30:57 root Exp $
*/
#if !defined( _CHARLIST_H )
#define _CHARLIST_H
typedef struct tagCharElem {
	void* pWho;
	struct tagCharElem* pNext;
	int nTimer;
	int nIntData;
} CharElem;

int IsInList( CharElem* pElem, void* pWho );
CharElem* InsertInList( CharElem** pElem, void* pWho, int nTimer );
CharElem* InsertInListInt( CharElem** pElem, void* pWho, int nTimer,
						   int nData );
int GetIntData( CharElem* pElem, void* pWho );
void SetIntData( CharElem** pElem, void* pWho, int nData, int iTimer = 0 );
int AddIntData( CharElem** pElem, void* pWho, int nData, int iTimer = 0 );
int SumIntData( CharElem* pElem, void* pWho = nullptr );
int SumIntDataPos( CharElem* pElem, void* pWho =nullptr );
int SumIntDataNeg( CharElem* pElem, void* pWho = nullptr );
void UpdateList( CharElem** pElem );
void RemoveFromList( CharElem** pElem, void* pWho );
void FreeList( CharElem** pElem );

#endif
