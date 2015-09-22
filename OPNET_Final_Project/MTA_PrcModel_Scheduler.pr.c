/* Process model C form file: MTA_PrcModel_Scheduler.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char MTA_PrcModel_Scheduler_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A modeler 7 51CDDB54 51CDDB54 1 Ramyad-PC Ideapad 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                                        ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

/* packet stream definitions */
#define MXT1_OUT_STRM			0
#define MXT2_OUT_STRM			1
#define MXT3_OUT_STRM			2
#define MXT4_OUT_STRM			3
#define MXT5_OUT_STRM			4
#define MXT6_OUT_STRM			5


#define RCV1_IN_STRM			0
#define RCV2_IN_STRM			1
#define RCV3_IN_STRM			2
#define RCV4_IN_STRM			3
#define RCV5_IN_STRM			4
#define RCV6_IN_STRM			5
#define POLL_GEN_IN_STRM		6
#define QUEUE_CHECK_GEN_IN_STRM	7

/* transition macros */
#define POLLING (op_intrpt_type() == \
	OPC_INTRPT_STRM && op_intrpt_strm() == POLL_GEN_IN_STRM)

#define POLL_RCV (op_intrpt_type() == \
	OPC_INTRPT_STRM && (op_intrpt_strm() == RCV1_IN_STRM || \
						op_intrpt_strm() == RCV2_IN_STRM || \
						op_intrpt_strm() == RCV3_IN_STRM || \
						op_intrpt_strm() == RCV4_IN_STRM || \
						op_intrpt_strm() == RCV5_IN_STRM || \
						op_intrpt_strm() == RCV6_IN_STRM))


/*packet formats*/
#define pkt_poll_poll_request 	0
#define pkt_poll_poll_answer 	1
#define pkt_poll_command		2
#define pkt_poll_hello_info		3

/* End of Header Block */

#if !defined (VOSD_NO_FIN)
#undef	BIN
#undef	BOUT
#define	BIN		FIN_LOCAL_FIELD(_op_last_line_passed) = __LINE__ - _op_block_origin;
#define	BOUT	BIN
#define	BINIT	FIN_LOCAL_FIELD(_op_last_line_passed) = 0; _op_block_origin = __LINE__;
#else
#define	BINIT
#endif /* #if !defined (VOSD_NO_FIN) */



/* State variable definitions */
typedef struct
	{
	/* Internal state tracking for FSM */
	FSM_SYS_STATE
	/* State Variables */
	int	                    		src_addr                                        ;	/* self address8 */
	int	                    		queue_size[7][7]                                ;	/* queue size for each clinet for each destination */
	int	                    		inout_client_link[7][3]                         ;	/* each node connected destinations */
	int	                    		LINK[12][2]                                     ;	/* This the network map::defined in init */
	int	                    		LINK_PAIRS[36][2]                               ;	/* These are possible LINK Pairs for activation */
	int	                    		LINK_PAIR_W[64]                                 ;	/* link pair weights */
	} MTA_PrcModel_Scheduler_state;

#define src_addr                		op_sv_ptr->src_addr
#define queue_size              		op_sv_ptr->queue_size
#define inout_client_link       		op_sv_ptr->inout_client_link
#define LINK                    		op_sv_ptr->LINK
#define LINK_PAIRS              		op_sv_ptr->LINK_PAIRS
#define LINK_PAIR_W             		op_sv_ptr->LINK_PAIR_W

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	MTA_PrcModel_Scheduler_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((MTA_PrcModel_Scheduler_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif

static void queue_check_and_command()
	{
	Packet *pkptr;
	int command_needed;
	int i,j,sum_queue;
	int a,b,c,d;
	int max1,max2;
	int w1,w2;
	int flag1,flag2_1,flag2_2;
	int Qos_w=1;
	FIN(command());
	
	//determine if clinets have datas	
	command_needed = 0;
	sum_queue = 0;
	
	for (i=0; i<7; i++) {
		for (j=0; j<7; j++) {
			sum_queue += queue_size[j][i];
			if (sum_queue > 1) { command_needed = 1; break; }
		}
	}
	
	
	//we need to command clinets to send datas
	if (command_needed == 1)
		{					
		//calculates weigths
		for (i=0;i<36;i++) 
			{
			a=LINK[LINK_PAIRS[i][0]][0]; 	//i pairs: first link start node
			b=LINK[LINK_PAIRS[i][0]][1];	//end node
				
			c=LINK[LINK_PAIRS[i][1]][0];	//i pairs: second link start node
			d=LINK[LINK_PAIRS[i][1]][1];	//end node
				
			max1=0;
			max2=0;
			for (j=1;j<7;j++)
				{
					Qos_w=1;
					//if (j==5) {Qos_w=2;}	//uncomment this part for QoS
											//you can add more for more Qos s
					
					w1=queue_size[a][j];
					w2=queue_size[b][j];
					if ((w1-w2)>max1) { max1 = (w1-w2) * Qos_w; }
					
					w1=queue_size[c][j];
					w2=queue_size[d][j];
					if ((w1-w2)>max2) { max2 = (w1-w2) * Qos_w; }					
				}
			LINK_PAIR_W[i] = max1+max2;
			}
			
		
		//now find bigger pair weight
		max1=0;
		flag1=0;
		for (i=0;i<64;i++)
			{
				if (LINK_PAIR_W[i] >= max1)
					{
						max1 = LINK_PAIR_W[i];
						flag1 = i;
					}
			}
		//the index is in flag1	
		a=LINK[LINK_PAIRS[flag1][0]][0]; 	//i pairs: first link start node
		b=LINK[LINK_PAIRS[flag1][0]][1];	//end node
				
		c=LINK[LINK_PAIRS[flag1][1]][0];	//i pairs: second link start node
		d=LINK[LINK_PAIRS[flag1][1]][1];	//end node
		
		flag2_1=0;
		flag2_2=0;
		max1=0;
		max2=0;
		for (j=1;j<7;j++)
			{
					w1=queue_size[a][j];
					w2=queue_size[b][j];
					if ((w1-w2)>max1) { max1 = w1-w2; flag2_1=j; }
					
					w1=queue_size[c][j];
					w2=queue_size[d][j];
					if ((w1-w2)>max2) { max2 = w1-w2; flag2_2=j; }					
			}
		
		//flag2_1 and flag2_2 is desired queues
		//send commands for nodes
		if (max1 != 0)
			{
			pkptr = op_pk_create_fmt("MTA_pkt_poll");
			op_pk_nfd_set_int32 (pkptr, "src_addr", 0);
			op_pk_nfd_set_int32 (pkptr, "dest_addr", a);
			op_pk_nfd_set_int32 (pkptr, "command_type",pkt_poll_command);
			op_pk_nfd_set_int32 (pkptr, "queue_sending_addr",b);
			op_pk_nfd_set_int32 (pkptr, "queue_dest_addr",flag2_1);
			switch (a)
				{
				case 1:	op_pk_send (pkptr, MXT1_OUT_STRM);
						break;
				case 2:	op_pk_send (pkptr, MXT2_OUT_STRM);
						break;
				case 3:	op_pk_send (pkptr, MXT3_OUT_STRM);
						break;
				case 4:	op_pk_send (pkptr, MXT4_OUT_STRM);
						break;
				case 5:	op_pk_send (pkptr, MXT5_OUT_STRM);
						break;
				case 6:	op_pk_send (pkptr, MXT6_OUT_STRM);
						break;
				}
					
			}
		if (max2 != 0)
			{
			pkptr = op_pk_create_fmt("MTA_pkt_poll");
			op_pk_nfd_set_int32 (pkptr, "src_addr", 0);
			op_pk_nfd_set_int32 (pkptr, "dest_addr", c);
			op_pk_nfd_set_int32 (pkptr, "command_type",pkt_poll_command);
			op_pk_nfd_set_int32 (pkptr, "queue_sending_addr",d);
			op_pk_nfd_set_int32 (pkptr, "queue_dest_addr",flag2_2);
			switch (c)
				{
				case 1:	op_pk_send (pkptr, MXT1_OUT_STRM);
						break;
				case 2:	op_pk_send (pkptr, MXT2_OUT_STRM);
						break;
				case 3:	op_pk_send (pkptr, MXT3_OUT_STRM);
						break;
				case 4:	op_pk_send (pkptr, MXT4_OUT_STRM);
						break;
				case 5:	op_pk_send (pkptr, MXT5_OUT_STRM);
						break;
				case 6:	op_pk_send (pkptr, MXT6_OUT_STRM);
						break;
				}		
			}		
		}
	
	FOUT;
	}

static void poll_rcv(void)
	{
	Packet *pkptr;
	int flag;
	int pkt_src_addr,pkt_dest_addr,command_type;
	int pkt_link_no,pkt_link_dest;
	int temp_queue_size;
	FIN(poll_rcv());
	
	//Determine The Input Port and get the pkt;
	if(op_intrpt_strm() == RCV1_IN_STRM)
		{
			 	pkptr = op_pk_get (RCV1_IN_STRM);
				flag = 1;
		}
	else if(op_intrpt_strm() == RCV2_IN_STRM)
		{
			 	pkptr = op_pk_get (RCV2_IN_STRM);
				flag = 2;
		}
	else if(op_intrpt_strm() == RCV3_IN_STRM)
		{
			 	pkptr = op_pk_get (RCV3_IN_STRM);
				flag = 3;
		}
	else if(op_intrpt_strm() == RCV4_IN_STRM)
		{
			 	pkptr = op_pk_get (RCV4_IN_STRM);
				flag = 4;
		}
	else if(op_intrpt_strm() == RCV5_IN_STRM)
		{
			 	pkptr = op_pk_get (RCV5_IN_STRM);
				flag = 5;
		}
	else if(op_intrpt_strm() == RCV6_IN_STRM)
		{
			 	pkptr = op_pk_get (RCV6_IN_STRM);
				flag = 6;
		}
	
	//now check more on fields
	op_pk_nfd_get_int32 (pkptr, "dest_addr", &pkt_dest_addr);
	op_pk_nfd_get_int32 (pkptr, "src_addr", &pkt_src_addr);
	op_pk_nfd_get_int32 (pkptr, "command_type", &command_type);
	if (pkt_dest_addr !=0)
		{
		op_pk_destroy (pkptr);
		FOUT;
		}
	
	//update queue_size
	if(command_type == pkt_poll_poll_answer)
		{
	op_pk_nfd_get_int32 (pkptr, "queue_size_1", &temp_queue_size);
	queue_size[pkt_src_addr][1] = temp_queue_size;
	op_pk_nfd_get_int32 (pkptr, "queue_size_2", &temp_queue_size);
	queue_size[pkt_src_addr][2] = temp_queue_size;
	op_pk_nfd_get_int32 (pkptr, "queue_size_3", &temp_queue_size);
	queue_size[pkt_src_addr][3] = temp_queue_size;
	op_pk_nfd_get_int32 (pkptr, "queue_size_4", &temp_queue_size);
	queue_size[pkt_src_addr][4] = temp_queue_size;
	op_pk_nfd_get_int32 (pkptr, "queue_size_5", &temp_queue_size);
	queue_size[pkt_src_addr][5] = temp_queue_size;
	op_pk_nfd_get_int32 (pkptr, "queue_size_6", &temp_queue_size);
	queue_size[pkt_src_addr][6] = temp_queue_size;
		}
	
	//update inout_links
	else if (command_type == pkt_poll_hello_info)
		{
		op_pk_nfd_get_int32 (pkptr, "link_no", &pkt_link_no);
		op_pk_nfd_get_int32 (pkptr, "link_dest", &pkt_link_dest);
		if(pkt_src_addr<1 || pkt_src_addr>7) { FOUT; }
		inout_client_link[pkt_src_addr][pkt_link_no] = pkt_link_dest;
		}
	
	//op_pk_destroy (pkptr);
	FOUT;
	}

static void poll(void)
	{
	Packet *pkptr;
	FIN(poll());
	
	//get and destroy recevied packet
	pkptr = op_pk_get(POLL_GEN_IN_STRM);
	op_pk_destroy(pkptr);
	
	/*now generate a poll packet for all nodes*/
	//for clinet1
	pkptr = op_pk_create_fmt("MTA_pkt_poll");
	op_pk_nfd_set_int32 (pkptr, "src_addr", 0);
	op_pk_nfd_set_int32 (pkptr, "dest_addr", 1);
	op_pk_nfd_set_int32 (pkptr, "command_type",pkt_poll_poll_request);
	op_pk_send (pkptr, MXT1_OUT_STRM);
	//for clinet2
	pkptr = op_pk_create_fmt("MTA_pkt_poll");
	op_pk_nfd_set_int32 (pkptr, "src_addr", 0);
	op_pk_nfd_set_int32 (pkptr, "dest_addr", 2);
	op_pk_nfd_set_int32 (pkptr, "command_type",pkt_poll_poll_request);
	op_pk_send (pkptr, MXT2_OUT_STRM);
	//for clinet3
	pkptr = op_pk_create_fmt("MTA_pkt_poll");
	op_pk_nfd_set_int32 (pkptr, "src_addr", 0);
	op_pk_nfd_set_int32 (pkptr, "dest_addr", 3);
	op_pk_nfd_set_int32 (pkptr, "command_type",pkt_poll_poll_request);
	op_pk_send (pkptr, MXT3_OUT_STRM);
	//for clinet4
	pkptr = op_pk_create_fmt("MTA_pkt_poll");
	op_pk_nfd_set_int32 (pkptr, "src_addr", 0);
	op_pk_nfd_set_int32 (pkptr, "dest_addr", 4);
	op_pk_nfd_set_int32 (pkptr, "command_type",pkt_poll_poll_request);
	op_pk_send (pkptr, MXT4_OUT_STRM);
	//for clinet5
	pkptr = op_pk_create_fmt("MTA_pkt_poll");
	op_pk_nfd_set_int32 (pkptr, "src_addr", 0);
	op_pk_nfd_set_int32 (pkptr, "dest_addr", 5);
	op_pk_nfd_set_int32 (pkptr, "command_type",pkt_poll_poll_request);
	op_pk_send (pkptr, MXT5_OUT_STRM);
	//for clinet6
	pkptr = op_pk_create_fmt("MTA_pkt_poll");
	op_pk_nfd_set_int32 (pkptr, "src_addr", 0);
	op_pk_nfd_set_int32 (pkptr, "dest_addr", 6);
	op_pk_nfd_set_int32 (pkptr, "command_type",pkt_poll_poll_request);
	op_pk_send (pkptr, MXT6_OUT_STRM);
	
	queue_check_and_command();

	FOUT;
	}

/* End of Function Block */

/* Undefine optional tracing in FIN/FOUT/FRET */
/* The FSM has its own tracing code and the other */
/* functions should not have any tracing.		  */
#undef FIN_TRACING
#define FIN_TRACING

#undef FOUTRET_TRACING
#define FOUTRET_TRACING

#if defined (__cplusplus)
extern "C" {
#endif
	void MTA_PrcModel_Scheduler (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_MTA_PrcModel_Scheduler_init (int * init_block_ptr);
	void _op_MTA_PrcModel_Scheduler_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_MTA_PrcModel_Scheduler_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_MTA_PrcModel_Scheduler_alloc (VosT_Obtype, int);
	void _op_MTA_PrcModel_Scheduler_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
MTA_PrcModel_Scheduler (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (MTA_PrcModel_Scheduler ());

		{
		/* Temporary Variables */
		int i;
		int j;
		/* End of Temporary Variables */


		FSM_ENTER ("MTA_PrcModel_Scheduler")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (0, "init", "MTA_PrcModel_Scheduler [init enter execs]")
				FSM_PROFILE_SECTION_IN ("MTA_PrcModel_Scheduler [init enter execs]", state0_enter_exec)
				{
				op_ima_obj_attr_get(op_id_self(), "Address", &src_addr); //get the self address
				for (i=0; i<7; i++) {
					for (j=0; j<7; j++) {
					queue_size[j][i]=0;
					}
				}
				for (i=0; i<7; i++) {
					for (j=0; j<3; j++) {
					inout_client_link[j][i]=-1;
					}
				}
				
				//******NETWORK MAP
				LINK[0][0]=  1 ; LINK[0][1]=  2 ;
				LINK[1][0]=  2 ; LINK[1][1]=  1 ;
				LINK[2][0]=  2 ; LINK[2][1]=  3 ;
				LINK[3][0]=  3 ; LINK[3][1]=  2 ;
				LINK[4][0]=  3 ; LINK[4][1]=  4 ;
				LINK[5][0]=  4 ; LINK[5][1]=  3 ;
				LINK[6][0]=  4 ; LINK[6][1]=  5 ;
				LINK[7][0]=  5 ; LINK[7][1]=  4 ;
				
				LINK[8][0]=  5 ; LINK[8][1]=  6 ;
				LINK[9][0]=	 6 ; LINK[9][1]=  5 ;
				//LINK[8][0]=  3 ; LINK[8][1]=  4 ; //for failure in node 5 and 6
				//LINK[9][0]=	 4 ; LINK[9][1]=  5 ;
				
				LINK[10][0]= 6 ; LINK[10][1]= 2 ;
				LINK[11][0]= 2 ; LINK[11][1]= 6 ;
				
				
				//********POSSIBLE LINK PAIRS
				LINK_PAIRS[0][0] = 0 ; LINK_PAIRS[0][1] = 4;
				LINK_PAIRS[1][0] = 0 ; LINK_PAIRS[1][1] = 5;
				LINK_PAIRS[2][0] = 0 ; LINK_PAIRS[2][1] = 6;
				LINK_PAIRS[3][0] = 0 ; LINK_PAIRS[3][1] = 7;
				LINK_PAIRS[4][0] = 0 ; LINK_PAIRS[4][1] = 8;
				LINK_PAIRS[5][0] = 0 ; LINK_PAIRS[5][1] = 9;
				
				LINK_PAIRS[6][0] = 1 ; LINK_PAIRS[6][1] = 4;
				LINK_PAIRS[7][0] = 1 ; LINK_PAIRS[7][1] = 5;
				LINK_PAIRS[8][0] = 1 ; LINK_PAIRS[8][1] = 6;
				LINK_PAIRS[9][0] = 1 ; LINK_PAIRS[9][1] = 7;
				LINK_PAIRS[10][0] = 1 ; LINK_PAIRS[10][1] = 8;
				LINK_PAIRS[11][0] = 1 ; LINK_PAIRS[11][1] = 9;
				
				LINK_PAIRS[12][0] = 2 ; LINK_PAIRS[12][1] = 6;
				LINK_PAIRS[13][0] = 2 ; LINK_PAIRS[13][1] = 7;
				LINK_PAIRS[14][0] = 2 ; LINK_PAIRS[14][1] = 8;
				LINK_PAIRS[15][0] = 2 ; LINK_PAIRS[15][1] = 9;
				
				LINK_PAIRS[16][0] = 3 ; LINK_PAIRS[16][1] = 6;
				LINK_PAIRS[17][0] = 3 ; LINK_PAIRS[17][1] = 7;
				LINK_PAIRS[18][0] = 3 ; LINK_PAIRS[18][1] = 8;
				LINK_PAIRS[19][0] = 3 ; LINK_PAIRS[19][1] = 9;
				
				LINK_PAIRS[20][0] = 10 ; LINK_PAIRS[20][1] = 4;
				LINK_PAIRS[21][0] = 10 ; LINK_PAIRS[21][1] = 5;
				LINK_PAIRS[22][0] = 10 ; LINK_PAIRS[22][1] = 6;
				LINK_PAIRS[23][0] = 10 ; LINK_PAIRS[23][1] = 7;
				
				LINK_PAIRS[24][0] = 11 ; LINK_PAIRS[24][1] = 4;
				LINK_PAIRS[25][0] = 11 ; LINK_PAIRS[25][1] = 5;
				LINK_PAIRS[26][0] = 11 ; LINK_PAIRS[26][1] = 6;
				LINK_PAIRS[27][0] = 11 ; LINK_PAIRS[27][1] = 7;
				
				LINK_PAIRS[28][0] = 4 ; LINK_PAIRS[28][1] = 8;
				LINK_PAIRS[29][0] = 4 ; LINK_PAIRS[29][1] = 9;
				LINK_PAIRS[30][0] = 4 ; LINK_PAIRS[30][1] = 10;
				LINK_PAIRS[31][0] = 4 ; LINK_PAIRS[31][1] = 11;
				
				LINK_PAIRS[32][0] = 5 ; LINK_PAIRS[32][1] = 8;
				LINK_PAIRS[33][0] = 5 ; LINK_PAIRS[33][1] = 9;
				LINK_PAIRS[34][0] = 5 ; LINK_PAIRS[34][1] = 10;
				LINK_PAIRS[35][0] = 5 ; LINK_PAIRS[35][1] = 11;
				
				
				//*********LINK PAIR WEIGHTS
				for (i=0;i<64;i++) {
					LINK_PAIR_W[i] = 0;
				}
				}
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (0, "init", "MTA_PrcModel_Scheduler [init exit execs]")


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "idle", "tr_10", "MTA_PrcModel_Scheduler [init -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle", state1_enter_exec, "MTA_PrcModel_Scheduler [idle enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"MTA_PrcModel_Scheduler")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "MTA_PrcModel_Scheduler [idle exit execs]")


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("MTA_PrcModel_Scheduler [idle trans conditions]", state1_trans_conds)
			FSM_INIT_COND (POLLING)
			FSM_TEST_COND (POLL_RCV)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state1_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 1, state1_enter_exec, poll();, "POLLING", "poll()", "idle", "idle", "tr_3", "MTA_PrcModel_Scheduler [idle -> idle : POLLING / poll()]")
				FSM_CASE_TRANSIT (1, 1, state1_enter_exec, poll_rcv();, "POLL_RCV", "poll_rcv()", "idle", "idle", "tr_6", "MTA_PrcModel_Scheduler [idle -> idle : POLL_RCV / poll_rcv()]")
				FSM_CASE_TRANSIT (2, 1, state1_enter_exec, ;, "default", "", "idle", "idle", "tr_8", "MTA_PrcModel_Scheduler [idle -> idle : default / ]")
				}
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"MTA_PrcModel_Scheduler")
		}
	}




void
_op_MTA_PrcModel_Scheduler_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_MTA_PrcModel_Scheduler_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_MTA_PrcModel_Scheduler_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_MTA_PrcModel_Scheduler_svar function. */
#undef src_addr
#undef queue_size
#undef inout_client_link
#undef LINK
#undef LINK_PAIRS
#undef LINK_PAIR_W

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_MTA_PrcModel_Scheduler_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_MTA_PrcModel_Scheduler_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (MTA_PrcModel_Scheduler)",
		sizeof (MTA_PrcModel_Scheduler_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_MTA_PrcModel_Scheduler_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	MTA_PrcModel_Scheduler_state * ptr;
	FIN_MT (_op_MTA_PrcModel_Scheduler_alloc (obtype))

	ptr = (MTA_PrcModel_Scheduler_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "MTA_PrcModel_Scheduler [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_MTA_PrcModel_Scheduler_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	MTA_PrcModel_Scheduler_state		*prs_ptr;

	FIN_MT (_op_MTA_PrcModel_Scheduler_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (MTA_PrcModel_Scheduler_state *)gen_ptr;

	if (strcmp ("src_addr" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->src_addr);
		FOUT
		}
	if (strcmp ("queue_size" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->queue_size);
		FOUT
		}
	if (strcmp ("inout_client_link" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->inout_client_link);
		FOUT
		}
	if (strcmp ("LINK" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->LINK);
		FOUT
		}
	if (strcmp ("LINK_PAIRS" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->LINK_PAIRS);
		FOUT
		}
	if (strcmp ("LINK_PAIR_W" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->LINK_PAIR_W);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

