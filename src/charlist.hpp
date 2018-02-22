/*$Id: charlist.h,v 1.2 2002/02/13 12:30:57 root Exp $
*/
#ifndef __CHARLIST_HPP
#define __CHARLIST_HPP
typedef struct tagCharElem {
	void* pWho;
	struct tagCharElem* pNext;
	int nTimer;
	int nIntData;
} CharElem;
int AddIntData( CharElem** pElem, void* pWho, int nData, int iTimer ) ;
void FreeList( CharElem** pElem ) ;
int GetIntData( CharElem* pElem, void* pWho ) ;
CharElem* InsertInList( CharElem** pElem, void* pWho, int nTimer ) ;
CharElem* InsertInListInt( CharElem** pElem, void* pWho, int nTimer,int nData );
int IsInList( CharElem* pElem, void* pWho ) ;
void RemoveFromList( CharElem** pElem, void* pWho ) ;
void SetIntData( CharElem** pElem, void* pWho, int nData, int iTimer ) ;
int SumIntData( CharElem* pElem, void* pWho ) ;
int SumIntDataNeg( CharElem* pElem, void* pWho ) ;
int SumIntDataPos( CharElem* pElem, void* pWho ) ;
void UpdateList( CharElem** pElem ) ;
#endif // __CHARLIST_HPP
