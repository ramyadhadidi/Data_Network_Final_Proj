/* Process model C form file: MTA_PrcModel_Clinet.pr.c */
/* Portions of this file copyright 1986-2008 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char MTA_PrcModel_Clinet_pr_c [] = "MIL_3_Tfile_Hdr_ 145A 30A op_runsim 7 51CDC983 51CDC983 1 Ramyad-PC Ideapad 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                                      ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

/* packet stream definitions */
#define MXT0_OUT_STRM			0
#define MXT1_OUT_STRM			1
#define MXT2_OUT_STRM			2
#define MXT_SCH_OUT_STRM		3

#define GENERATOR_IN_STRM		0
#define RCV0_IN_STRM			1
#define RCV1_IN_STRM			2
#define RCV2_IN_STRM			3
#define RCV_SCH_IN_STRM			4

/* transition macros */
#define POLL_RCV (op_intrpt_type() == \
	OPC_INTRPT_STRM && op_intrpt_strm() == RCV_SCH_IN_STRM)
#define DATA_RCV (op_intrpt_type() == \
	OPC_INTRPT_STRM && (op_intrpt_strm() == RCV0_IN_STRM || \
						op_intrpt_strm() == RCV1_IN_STRM || \
						op_intrpt_strm() == RCV2_IN_STRM))
#define GENERATOR_RCV (op_intrpt_type() == \
	OPC_INTRPT_STRM && op_intrpt_strm() == GENERATOR_IN_STRM)

/*packet formats*/
#define pkt_poll_poll_request 	0
#define pkt_poll_poll_answer 	1
#define pkt_poll_command		2
#define pkt_poll_hello_info		3

static void send_hellos();

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
	int	                    		src_addr                                        ;	/* node self address */
	int	                    		queue_size[7]                                   ;	/* queue size */
	int	                    		mxt_address[3]                                  ;	/* save wich address maps to which mxt */
	int	                    		flow_dest[2]                                    ;	/* flow is directed to which address */
	int	                    		flow_sender[2]                                  ;	/* flow senders are who */
	int	                    		flow_dest_size                                  ;	/* size of above array */
	int	                    		flow_sender_size                                ;	/* size of above array */
	int	                    		hello_addr                                      ;	/* an address for hello packets */
	Stathandle	             		ete_gsh                                         ;	/* for delay calculation */
	Stathandle	             		ete_hop_count                                   ;	/* for hop count statistics */
	int	                    		ID                                              ;	/* a static varible to distinguish between pkt */
	} MTA_PrcModel_Clinet_state;

#define src_addr                		op_sv_ptr->src_addr
#define queue_size              		op_sv_ptr->queue_size
#define mxt_address             		op_sv_ptr->mxt_address
#define flow_dest               		op_sv_ptr->flow_dest
#define flow_sender             		op_sv_ptr->flow_sender
#define flow_dest_size          		op_sv_ptr->flow_dest_size
#define flow_sender_size        		op_sv_ptr->flow_sender_size
#define hello_addr              		op_sv_ptr->hello_addr
#define ete_gsh                 		op_sv_ptr->ete_gsh
#define ete_hop_count           		op_sv_ptr->ete_hop_count
#define ID                      		op_sv_ptr->ID

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	MTA_PrcModel_Clinet_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((MTA_PrcModel_Clinet_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif

static void out_data(void)
	{
	Packet *pkptr;
	int flag;
	int i;
	FIN(out_data());
	
	//get the packet
	pkptr = op_pk_get (GENERATOR_IN_STRM);
	op_pk_destroy(pkptr);
	//check if this a sender node	
	flag=-1;
	for (i=0; i<flow_sender_size; i++) {
		if (src_addr == flow_sender[i]) {
			flag=i;
		}
	}
	if(flag==-1) {FOUT;}
	
	//now it is a sender
	//so change the packet and save it to the queues
	pkptr = op_pk_create_fmt ("MTA_pkt_data");
	op_pk_nfd_set_int32 (pkptr, "src_addr", src_addr);
	op_pk_nfd_set_int32 (pkptr, "dest_addr", flow_dest[flag]);
	op_pk_nfd_set_int32 (pkptr, "hop_count", 0);
	op_pk_nfd_set_int32 (pkptr, "pkt_ID", ID);
	ID++;
	
	//printf("\nGenerate DATA:%d-->%d\n",src_addr, flow_dest[flag]);
	
	//insert to queue 		
	op_subq_pk_insert(flow_dest[flag], pkptr, OPC_QPOS_HEAD);
	queue_size[flow_dest[flag]]++;
	
	FOUT;
	}

static void in_data(void)
	{
	Packet *pkptr;
	int flag;
	int pkt_dest_addr,pkt_src_addr;
	double ete_delay;
	int hop_count_temp;
	FIN(in_data());
	
	//Determine The Input Port and get the pkt;
	if(op_intrpt_strm() == RCV0_IN_STRM)
		{
			 	pkptr = op_pk_get (RCV0_IN_STRM);
				flag = 0;
		}
	else if(op_intrpt_strm() == RCV1_IN_STRM)
		{
			 	pkptr = op_pk_get (RCV1_IN_STRM);
				flag = 1;
		}
	else if(op_intrpt_strm() == RCV2_IN_STRM)
		{
			 	pkptr = op_pk_get (RCV2_IN_STRM);
				flag = 2;
		}
	
	//now check more on fields
	op_pk_nfd_get_int32 (pkptr, "dest_addr", &pkt_dest_addr);
	op_pk_nfd_get_int32 (pkptr, "src_addr", &pkt_src_addr);
	
	if (pkt_dest_addr == 999)
		{
		//this is a hello packet, MAP IT!
		mxt_address[flag] = pkt_src_addr;
		op_pk_destroy (pkptr);
		
		//now generate a hello_info packet for the scheduler
		pkptr = op_pk_create_fmt ("MTA_pkt_poll");
		op_pk_nfd_set_int32 (pkptr, "src_addr", src_addr);
		op_pk_nfd_set_int32 (pkptr, "dest_addr", 0);
		op_pk_nfd_set_int32 (pkptr, "link_no", flag);
		op_pk_nfd_set_int32 (pkptr, "link_dest", pkt_src_addr);
		op_pk_nfd_set_int32 (pkptr, "command_type", pkt_poll_hello_info);
		op_pk_send (pkptr,MXT_SCH_OUT_STRM);
		
		FOUT;
		}
	
	//printf("\nPacket Recieve: %d--->%d dest:%d",pkt_src_addr,src_addr,pkt_dest_addr);
	
	//other packets so check them if they reach to the destination
	if (pkt_dest_addr == src_addr)
		{

		ete_delay = op_sim_time () - op_pk_creation_time_get (pkptr);
		op_stat_write (ete_gsh, ete_delay);
		
		op_pk_nfd_get_int32 (pkptr, "hop_count", &hop_count_temp);
		hop_count_temp++;
		op_stat_write (ete_hop_count, hop_count_temp);
		
		op_pk_destroy (pkptr);
		FOUT;
		}
	
	//if not, increse the hop count and put them in queues
	//insert to queue
	op_pk_nfd_get_int32 (pkptr, "hop_count", &hop_count_temp);
	hop_count_temp++;
	op_pk_nfd_set_int32 (pkptr, "hop_count", hop_count_temp);
	op_subq_pk_insert(pkt_dest_addr, pkptr, OPC_QPOS_HEAD);
	queue_size[pkt_dest_addr]++;
	
	FOUT;
	}

static void answer_to_poll(void)
	{
	Packet *pkptr;
	int pkt_dest_addr,pkt_src_addr,command_type;
	int pkt_queue_dest_addr,pkt_queue_sending_addr,pkt_ID;
	int flag,i;
	FIN(answer_to_poll());
	
	pkptr = op_pk_get (RCV_SCH_IN_STRM);
	op_pk_nfd_get_int32 (pkptr, "dest_addr", &pkt_dest_addr);
	op_pk_nfd_get_int32 (pkptr, "src_addr", &pkt_src_addr);
	op_pk_nfd_get_int32 (pkptr, "command_type", &command_type);
	if (pkt_dest_addr !=src_addr || pkt_src_addr != 0) 
		{//this is a bad sended packet or not from schueler, dont answer
		op_pk_destroy(pkptr);
		FOUT;
		}
	
	//a poll request, send queue sizes
	if (command_type == pkt_poll_poll_request)
		{
		op_pk_destroy(pkptr);
		pkptr = op_pk_create_fmt("MTA_pkt_poll");
		op_pk_nfd_set_int32 (pkptr, "src_addr", src_addr);
		op_pk_nfd_set_int32 (pkptr, "dest_addr", 0);
		op_pk_nfd_set_int32 (pkptr, "queue_size_1", queue_size[1]);
		op_pk_nfd_set_int32 (pkptr, "queue_size_2", queue_size[2]);
		op_pk_nfd_set_int32 (pkptr, "queue_size_3", queue_size[3]);
		op_pk_nfd_set_int32 (pkptr, "queue_size_4", queue_size[4]);
		op_pk_nfd_set_int32 (pkptr, "queue_size_5", queue_size[5]);
		op_pk_nfd_set_int32 (pkptr, "queue_size_6", queue_size[6]);
		op_pk_nfd_set_int32 (pkptr, "command_type", pkt_poll_poll_answer);
		op_pk_send (pkptr,MXT_SCH_OUT_STRM);
		}
	
	//a command, send a packet, update queue sizes
	if (command_type == pkt_poll_command) 
		{		
		
		op_pk_nfd_get_int32 (pkptr, "queue_dest_addr", &pkt_queue_dest_addr);
		op_pk_nfd_get_int32 (pkptr, "queue_sending_addr", &pkt_queue_sending_addr);
		op_pk_destroy(pkptr);
		
		if (queue_size[pkt_queue_dest_addr] == 0) { FOUT; } //I have no data for it to send!
		flag=-1;
		for (i=0; i<3; i++) {
			if(mxt_address[i] == pkt_queue_sending_addr) {
			flag = i;
			}
		}
		if(flag==-1) { FOUT; } //I have no connection to it!
		//send data from queue
		//take out from queue
		pkptr = op_subq_pk_remove(pkt_queue_dest_addr, OPC_QPOS_TAIL);
		queue_size[pkt_queue_dest_addr]--;
		op_pk_nfd_get_int32 (pkptr, "pkt_ID", &pkt_ID);
		
		switch (flag)
			{
			case 0:	op_pk_send (pkptr,MXT0_OUT_STRM);
					break;
			case 1:	op_pk_send (pkptr,MXT1_OUT_STRM);
					break;
			case 2:	op_pk_send (pkptr,MXT2_OUT_STRM);
					break;
			}
		
		printf("\nPacket Rout: %d--->%d dest:%d(%d)",src_addr,pkt_queue_sending_addr,pkt_queue_dest_addr,pkt_ID);
		}
	
	FOUT;
	}

static void send_hellos() 
	{
	Packet* pkptr_temp;	
	FIN(send_hellos())
		
	pkptr_temp = op_pk_create_fmt("MTA_pkt_data");
	op_pk_nfd_set_int32 (pkptr_temp, "src_addr", src_addr);
	op_pk_nfd_set_int32 (pkptr_temp, "dest_addr", hello_addr);
	op_pk_send (pkptr_temp,MXT0_OUT_STRM);
	
	pkptr_temp = op_pk_create_fmt("MTA_pkt_data");
	op_pk_nfd_set_int32 (pkptr_temp, "src_addr", src_addr);
	op_pk_nfd_set_int32 (pkptr_temp, "dest_addr", hello_addr);
	op_pk_send (pkptr_temp,MXT1_OUT_STRM);
	
	pkptr_temp = op_pk_create_fmt("MTA_pkt_data");
	op_pk_nfd_set_int32 (pkptr_temp, "src_addr", src_addr);
	op_pk_nfd_set_int32 (pkptr_temp, "dest_addr", hello_addr);
	op_pk_send (pkptr_temp,MXT2_OUT_STRM);
	
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
	void MTA_PrcModel_Clinet (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Obtype _op_MTA_PrcModel_Clinet_init (int * init_block_ptr);
	void _op_MTA_PrcModel_Clinet_diag (OP_SIM_CONTEXT_ARG_OPT);
	void _op_MTA_PrcModel_Clinet_terminate (OP_SIM_CONTEXT_ARG_OPT);
	VosT_Address _op_MTA_PrcModel_Clinet_alloc (VosT_Obtype, int);
	void _op_MTA_PrcModel_Clinet_svar (void *, const char *, void **);


#if defined (__cplusplus)
} /* end of 'extern "C"' */
#endif




/* Process model interrupt handling procedure */


void
MTA_PrcModel_Clinet (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (MTA_PrcModel_Clinet ());

		{
		/* Temporary Variables */
		int i;
		/* End of Temporary Variables */


		FSM_ENTER ("MTA_PrcModel_Clinet")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (init) enter executives **/
			FSM_STATE_ENTER_FORCED_NOLABEL (0, "init", "MTA_PrcModel_Clinet [init enter execs]")
				FSM_PROFILE_SECTION_IN ("MTA_PrcModel_Clinet [init enter execs]", state0_enter_exec)
				{
				//************************************************
				//*******INIT
				
				//get the self address
				op_ima_obj_attr_get(op_id_self(), "Address", &src_addr); 
				
				//initialize
				hello_addr=999;
				for (i=0; i<7; i++) {
					queue_size[i] = 0;
				}
				for (i=0; i<3; i++) {
					mxt_address[i] = 0;
				}
				
				ID=0;
				
				//send hello packet for mapping mxt to dest addresse
				send_hellos();
				
				//************************************************
				//*******FLOW PROGRAMMING
				
				//detemine which node for sending
				flow_sender[0] = 2;
				flow_sender[1] = 6;
				flow_sender_size = 2;
				
				//determine which addresses for sending data
				flow_dest[0] = 5;
				flow_dest[1] = 4;
				flow_dest_size = 2;
				
				//************************************************
				//*******STATISTICS
				ete_gsh = op_stat_reg ("ETE Delay",OPC_STAT_INDEX_NONE,OPC_STAT_GLOBAL); 
				ete_hop_count = op_stat_reg ("ETE Hop Count",OPC_STAT_INDEX_NONE,OPC_STAT_GLOBAL);
				
				
				}
				FSM_PROFILE_SECTION_OUT (state0_enter_exec)

			/** state (init) exit executives **/
			FSM_STATE_EXIT_FORCED (0, "init", "MTA_PrcModel_Clinet [init exit execs]")


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "init", "idle", "tr_21", "MTA_PrcModel_Clinet [init -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (1, "idle", state1_enter_exec, "MTA_PrcModel_Clinet [idle enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"MTA_PrcModel_Clinet")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "idle", "MTA_PrcModel_Clinet [idle exit execs]")


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("MTA_PrcModel_Clinet [idle trans conditions]", state1_trans_conds)
			FSM_INIT_COND (POLL_RCV)
			FSM_TEST_COND (DATA_RCV)
			FSM_TEST_COND (GENERATOR_RCV)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state1_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 2, state2_enter_exec, ;, "POLL_RCV", "", "idle", "POLL_RCV", "tr_14", "MTA_PrcModel_Clinet [idle -> POLL_RCV : POLL_RCV / ]")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "DATA_RCV", "", "idle", "DATA_RCV", "tr_16", "MTA_PrcModel_Clinet [idle -> DATA_RCV : DATA_RCV / ]")
				FSM_CASE_TRANSIT (2, 4, state4_enter_exec, ;, "GENERATOR_RCV", "", "idle", "GEN_RCV", "tr_18", "MTA_PrcModel_Clinet [idle -> GEN_RCV : GENERATOR_RCV / ]")
				FSM_CASE_TRANSIT (3, 1, state1_enter_exec, ;, "default", "", "idle", "idle", "tr_3", "MTA_PrcModel_Clinet [idle -> idle : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (POLL_RCV) enter executives **/
			FSM_STATE_ENTER_FORCED (2, "POLL_RCV", state2_enter_exec, "MTA_PrcModel_Clinet [POLL_RCV enter execs]")
				FSM_PROFILE_SECTION_IN ("MTA_PrcModel_Clinet [POLL_RCV enter execs]", state2_enter_exec)
				{
				answer_to_poll();
				}
				FSM_PROFILE_SECTION_OUT (state2_enter_exec)

			/** state (POLL_RCV) exit executives **/
			FSM_STATE_EXIT_FORCED (2, "POLL_RCV", "MTA_PrcModel_Clinet [POLL_RCV exit execs]")


			/** state (POLL_RCV) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "POLL_RCV", "idle", "tr_15", "MTA_PrcModel_Clinet [POLL_RCV -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (DATA_RCV) enter executives **/
			FSM_STATE_ENTER_FORCED (3, "DATA_RCV", state3_enter_exec, "MTA_PrcModel_Clinet [DATA_RCV enter execs]")
				FSM_PROFILE_SECTION_IN ("MTA_PrcModel_Clinet [DATA_RCV enter execs]", state3_enter_exec)
				{
				in_data();
				}
				FSM_PROFILE_SECTION_OUT (state3_enter_exec)

			/** state (DATA_RCV) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "DATA_RCV", "MTA_PrcModel_Clinet [DATA_RCV exit execs]")


			/** state (DATA_RCV) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "DATA_RCV", "idle", "tr_17", "MTA_PrcModel_Clinet [DATA_RCV -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (GEN_RCV) enter executives **/
			FSM_STATE_ENTER_FORCED (4, "GEN_RCV", state4_enter_exec, "MTA_PrcModel_Clinet [GEN_RCV enter execs]")
				FSM_PROFILE_SECTION_IN ("MTA_PrcModel_Clinet [GEN_RCV enter execs]", state4_enter_exec)
				{
				out_data();
				}
				FSM_PROFILE_SECTION_OUT (state4_enter_exec)

			/** state (GEN_RCV) exit executives **/
			FSM_STATE_EXIT_FORCED (4, "GEN_RCV", "MTA_PrcModel_Clinet [GEN_RCV exit execs]")


			/** state (GEN_RCV) transition processing **/
			FSM_TRANSIT_FORCE (1, state1_enter_exec, ;, "default", "", "GEN_RCV", "idle", "tr_19", "MTA_PrcModel_Clinet [GEN_RCV -> idle : default / ]")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (0,"MTA_PrcModel_Clinet")
		}
	}




void
_op_MTA_PrcModel_Clinet_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}




void
_op_MTA_PrcModel_Clinet_terminate (OP_SIM_CONTEXT_ARG_OPT)
	{

	FIN_MT (_op_MTA_PrcModel_Clinet_terminate ())


	/* No Termination Block */

	Vos_Poolmem_Dealloc (op_sv_ptr);

	FOUT
	}


/* Undefine shortcuts to state variables to avoid */
/* syntax error in direct access to fields of */
/* local variable prs_ptr in _op_MTA_PrcModel_Clinet_svar function. */
#undef src_addr
#undef queue_size
#undef mxt_address
#undef flow_dest
#undef flow_sender
#undef flow_dest_size
#undef flow_sender_size
#undef hello_addr
#undef ete_gsh
#undef ete_hop_count
#undef ID

#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

VosT_Obtype
_op_MTA_PrcModel_Clinet_init (int * init_block_ptr)
	{
	VosT_Obtype obtype = OPC_NIL;
	FIN_MT (_op_MTA_PrcModel_Clinet_init (init_block_ptr))

	obtype = Vos_Define_Object_Prstate ("proc state vars (MTA_PrcModel_Clinet)",
		sizeof (MTA_PrcModel_Clinet_state));
	*init_block_ptr = 0;

	FRET (obtype)
	}

VosT_Address
_op_MTA_PrcModel_Clinet_alloc (VosT_Obtype obtype, int init_block)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	MTA_PrcModel_Clinet_state * ptr;
	FIN_MT (_op_MTA_PrcModel_Clinet_alloc (obtype))

	ptr = (MTA_PrcModel_Clinet_state *)Vos_Alloc_Object (obtype);
	if (ptr != OPC_NIL)
		{
		ptr->_op_current_block = init_block;
#if defined (OPD_ALLOW_ODB)
		ptr->_op_current_state = "MTA_PrcModel_Clinet [init enter execs]";
#endif
		}
	FRET ((VosT_Address)ptr)
	}



void
_op_MTA_PrcModel_Clinet_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	MTA_PrcModel_Clinet_state		*prs_ptr;

	FIN_MT (_op_MTA_PrcModel_Clinet_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (MTA_PrcModel_Clinet_state *)gen_ptr;

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
	if (strcmp ("mxt_address" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->mxt_address);
		FOUT
		}
	if (strcmp ("flow_dest" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->flow_dest);
		FOUT
		}
	if (strcmp ("flow_sender" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->flow_sender);
		FOUT
		}
	if (strcmp ("flow_dest_size" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->flow_dest_size);
		FOUT
		}
	if (strcmp ("flow_sender_size" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->flow_sender_size);
		FOUT
		}
	if (strcmp ("hello_addr" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->hello_addr);
		FOUT
		}
	if (strcmp ("ete_gsh" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->ete_gsh);
		FOUT
		}
	if (strcmp ("ete_hop_count" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->ete_hop_count);
		FOUT
		}
	if (strcmp ("ID" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->ID);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

