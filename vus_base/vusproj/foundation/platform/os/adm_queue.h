#ifndef _ADM_QUEUE_H_
#define _ADM_QUEUE_H_


#define pdFALSE			( ( D32S ) 0 )
#define pdTRUE			( ( D32S ) 1 )

#define pdPASS			( TRUE )
#define pdFAIL			( FALSE )
#define ADM_QUEUE_EMPTY	( ( D32S ) 0 )
#define ADM_QUEUE_FULL	( ( D32S ) 0 )

/* error definitions. */
#define errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY	( -1 )
#define errQUEUE_BLOCKED						( -4 )
#define errQUEUE_YIELD							( -5 )

/* For internal use only. */
#define	ADM_QUEUE_INSERT_TO_BACK		( ( D32U ) 0 )
#define	ADM_QUEUE_INSERT_TO_FRONT		( ( D32U ) 1 )
#define ADM_QUEUE_INSERT_OVERWRITE			( ( D32U ) 2 )

/* For internal use only.  These definitions *must* match those in queue.c. */
#define ADM_QUEUE_TYPE_BASE				( ( D8U ) 0U )
#define ADM_QUEUE_TYPE_SET					( ( D8U ) 0U )
#define ADM_QUEUE_TYPE_MUTEX 				( ( D8U ) 1U )
#define ADM_QUEUE_TYPE_COUNTING_SEMAPHORE	( ( D8U ) 2U )
#define ADM_QUEUE_TYPE_BINARY_SEMAPHORE	( ( D8U ) 3U )
#define ADM_QUEUE_TYPE_RECURSIVE_MUTEX		( ( D8U ) 4U )

/* Constants used with the cRxLock and cTxLock structure members. */
#define queueUNLOCKED					( ( D8S ) -1 )
#define queueLOCKED_UNMODIFIED			( ( D8S ) 0 )


typedef enum 
{
    ADM_TASK_MSGID_A2B = 0,                                 /* lowest, special for idle task */
	ADM_TASK_MSGID_B2C,
    ADM_TASK_MSGID_C2A,

    ADM_TASK_MSGID_OWND = 100,
    ADM_TASK_MSGID_OWND1 = 101,
    ADM_TASK_MSGID_OWND2 = 102,
} ADM_TASK_MSGID;

typedef struct _adm_queue_pointers_t
{
	D8S *pcTail;					/*< Points to the byte at the end of the queue storage area.  Once more byte is allocated than necessary to store the queue items, this is used as a marker. */
	D8S *pcReadFrom;				/*< Points to the last place that a queued item was read from when the structure is used as a queue. */
} adm_queue_pointers_t;

typedef struct _adm_queue_semaphore_data_t
{
	//TaskHandle_t xMutexHolder;		 /*< The handle of the task that holds the mutex. */
	D32U uxRecursiveCallCount;/*< Maintains a count of the number of times a recursive mutex has been recursively 'taken' when the structure is used as a mutex. */
} adm_queue_semaphore_data_t;

struct xLIST;
struct xLIST_ITEM
{
	struct xLIST_ITEM *pxNext;		/*< Pointer to the next ListItem_t in the list. */
	struct xLIST_ITEM *pxPrevious;	/*< Pointer to the previous ListItem_t in the list. */
	void * pvOwner;										/*< Pointer to the object (normally a TCB) that contains the list item.  There is therefore a two way link between the object containing the list item and the list item itself. */
	struct xLIST *pxContainer;		/*< Pointer to the list in which this list item is placed (if any). */
};
typedef struct xLIST_ITEM ListItem_t;					/* For some reason lint wants this as two separate definitions. */

struct xMINI_LIST_ITEM
{
	struct xLIST_ITEM *pxNext;
	struct xLIST_ITEM *pxPrevious;
};
typedef struct xMINI_LIST_ITEM MiniListItem_t;

typedef struct xLIST
{
	volatile D32U uxNumberOfItems;
	ListItem_t *pxIndex;			/*< Used to walk through the list.  Points to the last item returned by a call to listGET_OWNER_OF_NEXT_ENTRY (). */
	MiniListItem_t xListEnd;							/*< List item that contains the maximum possible item value meaning it is always at the end of the list and is therefore used as a marker. */
} List_t;



typedef struct _adm_queue_t
{
	D8S *pcHead;					/*< Points to the beginning of the queue storage area. */
	D8S *pcWriteTo;				/*< Points to the free next place in the storage area. */

	union
	{
		adm_queue_pointers_t xQueue;		/*< Data required exclusively when this structure is used as a queue. */
	} u;

	List_t xTasksWaitingToSend;		/*< List of tasks that are blocked waiting to post onto this queue.  Stored in priority order. */
	List_t xTasksWaitingToReceive;	/*< List of tasks that are blocked waiting to read from this queue.  Stored in priority order. */

	volatile D32U uxMessagesWaiting;/*< The number of items currently in the queue. */
	D32U uxLength;			/*< The length of the queue defined as the number of items it will hold, not the number of bytes. */
	D32U uxItemSize;			/*< The size of each items that the queue will hold. */

	volatile D8S cRxLock;		/*< Stores the number of items received from the queue (removed from the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */
	volatile D8S cTxLock;		/*< Stores the number of items transmitted to the queue (added to the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */

    struct _adm_queue_t *pxQueueSetContainer;

    D32U uxQueueNumber;
    D8U ucQueueType;

} adm_queue_t;

typedef struct _adm_queue_t *adm_queue_handle_t;

typedef struct _adm_message_t
{
    D32U msg_id;
    D32U msg_len;
    void *param;
} adm_thread_msg_t;

extern adm_handle_t adm_mutex_create(void);
extern void adm_mutex_lock(adm_handle_t inMutex);
extern void adm_mutex_unlock(adm_handle_t inMutex);
extern void adm_mutex_close(adm_handle_t inMutex);
extern D32S adm_thread_mutex_init(void);

extern adm_queue_handle_t adm_queue_generic_create( const D32U uxQueueLength, const D32S uxItemSize, const D8U ucQueueType );
extern D32S adm_queue_generic_send( adm_queue_handle_t xQueue, const void * const pvItemToQueue, /*TickType_t xTicksToWait,*/ const D32S xCopyPosition );
extern D32S adm_queue_generic_recv( adm_queue_handle_t xQueue, void * const pvBuffer/*, TickType_t xTicksToWait */, const D32U xJustPeeking );
extern void adm_thread_send_msg(adm_handle_t dstThreadHdl, D32U msgId, void* param, D32U len);

#define adm_queue_create( uxQueueLength, uxItemSize ) adm_queue_generic_create( uxQueueLength, uxItemSize, 0 )
#define adm_queue_send( xQueue, pvItemToQueue, xTicksToWait ) adm_queue_generic_send( ( xQueue ), ( pvItemToQueue ), /*( xTicksToWait ), */ADM_QUEUE_INSERT_TO_BACK )
#define adm_queue_receive( xQueue, pvBuffer, xTicksToWait ) adm_queue_generic_recv( ( xQueue ), ( pvBuffer ), /*( xTicksToWait ), */pdFALSE )


#endif
