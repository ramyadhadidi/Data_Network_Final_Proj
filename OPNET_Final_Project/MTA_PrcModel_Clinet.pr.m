MIL_3_Tfile_Hdr_ 145A 140A modeler 9 51C43992 51CDC97E E5 Ramyad-PC Ideapad 0 0 none none 0 0 none 462E7C99 3699 0 0 0 0 0 0 1e80 8                                                                                                                                                                                                                                                                                                                                                                                             ��g�      @   �   �  �  �  %�  1�  1�  1�  2�  2�  2�  4�  %}      Address    �������    ����           ����          ����          ����           �Z                 	   begsim intrpt         
   ����   
   doc file            	nd_module      endsim intrpt             ����      failure intrpts            disabled      intrpt interval         ԲI�%��}����      priority              ����      recovery intrpts            disabled      subqueue         
            count    ���   
   ����   
      list   	���   
            bit capacity   ���   
T�I�%��}����   
      pk capacity   ���   
T�I�%��}����   
         bit capacity   ���   
T�I�%��}����   
      pk capacity   ���   
T�I�%��}����   
         bit capacity   ���   
T�I�%��}����   
      pk capacity   ���   
T�I�%��}����   
         bit capacity   ���   
T�I�%��}����   
      pk capacity   ���   
T�I�%��}����   
         bit capacity   ���   
T�I�%��}����   
      pk capacity   ���   
T�I�%��}����   
         bit capacity   ���   
T�I�%��}����   
      pk capacity   ���   
T�I�%��}����   
         bit capacity   ���   
T�I�%��}����   
      pk capacity   ���   
T�I�%��}����   
         bit capacity   ���   
T�I�%��}����   
      pk capacity   ���   
T�I�%��}����   
   
   
   super priority             ����          !   /* node self address */   int	\src_addr;       /* queue size */   int	\queue_size[7];       )/* save wich address maps to which mxt */   int	\mxt_address[3];       '/* flow is directed to which address */   int	\flow_dest[2];       /* flow senders are who */   int	\flow_sender[2];       /* size of above array */   int	\flow_dest_size;       /* size of above array */   int	\flow_sender_size;       "/* an address for hello packets */   int	\hello_addr;       /* for delay calculation */   Stathandle	\ete_gsh;       /* for hop count statistics */   Stathandle	\ete_hop_count;       1/* a static varible to distinguish between pkt */   int	\ID;          int i;      /* packet stream definitions */   #define MXT0_OUT_STRM			0   #define MXT1_OUT_STRM			1   #define MXT2_OUT_STRM			2   #define MXT_SCH_OUT_STRM		3       #define GENERATOR_IN_STRM		0   #define RCV0_IN_STRM			1   #define RCV1_IN_STRM			2   #define RCV2_IN_STRM			3   #define RCV_SCH_IN_STRM			4       /* transition macros */   '#define POLL_RCV (op_intrpt_type() == \   8	OPC_INTRPT_STRM && op_intrpt_strm() == RCV_SCH_IN_STRM)   '#define DATA_RCV (op_intrpt_type() == \   :	OPC_INTRPT_STRM && (op_intrpt_strm() == RCV0_IN_STRM || \   +						op_intrpt_strm() == RCV1_IN_STRM || \   (						op_intrpt_strm() == RCV2_IN_STRM))   ,#define GENERATOR_RCV (op_intrpt_type() == \   :	OPC_INTRPT_STRM && op_intrpt_strm() == GENERATOR_IN_STRM)       /*packet formats*/    #define pkt_poll_poll_request 	0   #define pkt_poll_poll_answer 	1   #define pkt_poll_command		2   #define pkt_poll_hello_info		3       static void send_hellos();   �   static void out_data(void)   	{   	Packet *pkptr;   
	int flag;   	int i;   	FIN(out_data());   	   	//get the packet   '	pkptr = op_pk_get (GENERATOR_IN_STRM);   	op_pk_destroy(pkptr);   	//check if this a sender node	   		flag=-1;   %	for (i=0; i<flow_sender_size; i++) {   #		if (src_addr == flow_sender[i]) {   
			flag=i;   		}   	}   	if(flag==-1) {FOUT;}   	   	//now it is a sender   1	//so change the packet and save it to the queues   +	pkptr = op_pk_create_fmt ("MTA_pkt_data");   3	op_pk_nfd_set_int32 (pkptr, "src_addr", src_addr);   ;	op_pk_nfd_set_int32 (pkptr, "dest_addr", flow_dest[flag]);   -	op_pk_nfd_set_int32 (pkptr, "hop_count", 0);   +	op_pk_nfd_set_int32 (pkptr, "pkt_ID", ID);   	ID++;   	   A	//printf("\nGenerate DATA:%d-->%d\n",src_addr, flow_dest[flag]);   	   	//insert to queue 		   :	op_subq_pk_insert(flow_dest[flag], pkptr, OPC_QPOS_HEAD);   	queue_size[flow_dest[flag]]++;   	   	FOUT;   	}       static void in_data(void)   	{   	Packet *pkptr;   
	int flag;    	int pkt_dest_addr,pkt_src_addr;   	double ete_delay;   	int hop_count_temp;   	FIN(in_data());   	   ,	//Determine The Input Port and get the pkt;   %	if(op_intrpt_strm() == RCV0_IN_STRM)   		{   &			 	pkptr = op_pk_get (RCV0_IN_STRM);   				flag = 0;   		}   *	else if(op_intrpt_strm() == RCV1_IN_STRM)   		{   &			 	pkptr = op_pk_get (RCV1_IN_STRM);   				flag = 1;   		}   *	else if(op_intrpt_strm() == RCV2_IN_STRM)   		{   &			 	pkptr = op_pk_get (RCV2_IN_STRM);   				flag = 2;   		}   	   	//now check more on fields   :	op_pk_nfd_get_int32 (pkptr, "dest_addr", &pkt_dest_addr);   8	op_pk_nfd_get_int32 (pkptr, "src_addr", &pkt_src_addr);   	   	if (pkt_dest_addr == 999)   		{   #		//this is a hello packet, MAP IT!   #		mxt_address[flag] = pkt_src_addr;   		op_pk_destroy (pkptr);   		   6		//now generate a hello_info packet for the scheduler   ,		pkptr = op_pk_create_fmt ("MTA_pkt_poll");   4		op_pk_nfd_set_int32 (pkptr, "src_addr", src_addr);   .		op_pk_nfd_set_int32 (pkptr, "dest_addr", 0);   /		op_pk_nfd_set_int32 (pkptr, "link_no", flag);   9		op_pk_nfd_set_int32 (pkptr, "link_dest", pkt_src_addr);   C		op_pk_nfd_set_int32 (pkptr, "command_type", pkt_poll_hello_info);   &		op_pk_send (pkptr,MXT_SCH_OUT_STRM);   		   		FOUT;   		}   	   T	//printf("\nPacket Recieve: %d--->%d dest:%d",pkt_src_addr,src_addr,pkt_dest_addr);   	   ?	//other packets so check them if they reach to the destination   	if (pkt_dest_addr == src_addr)   		{       ?		ete_delay = op_sim_time () - op_pk_creation_time_get (pkptr);   %		op_stat_write (ete_gsh, ete_delay);   		   <		op_pk_nfd_get_int32 (pkptr, "hop_count", &hop_count_temp);   		hop_count_temp++;   0		op_stat_write (ete_hop_count, hop_count_temp);   		   		op_pk_destroy (pkptr);   		FOUT;   		}   	   7	//if not, increse the hop count and put them in queues   	//insert to queue   ;	op_pk_nfd_get_int32 (pkptr, "hop_count", &hop_count_temp);   	hop_count_temp++;   :	op_pk_nfd_set_int32 (pkptr, "hop_count", hop_count_temp);   8	op_subq_pk_insert(pkt_dest_addr, pkptr, OPC_QPOS_HEAD);   	queue_size[pkt_dest_addr]++;   	   	FOUT;   	}        static void answer_to_poll(void)   	{   	Packet *pkptr;   -	int pkt_dest_addr,pkt_src_addr,command_type;   7	int pkt_queue_dest_addr,pkt_queue_sending_addr,pkt_ID;   	int flag,i;   	FIN(answer_to_poll());   	   %	pkptr = op_pk_get (RCV_SCH_IN_STRM);   :	op_pk_nfd_get_int32 (pkptr, "dest_addr", &pkt_dest_addr);   8	op_pk_nfd_get_int32 (pkptr, "src_addr", &pkt_src_addr);   <	op_pk_nfd_get_int32 (pkptr, "command_type", &command_type);   4	if (pkt_dest_addr !=src_addr || pkt_src_addr != 0)    B		{//this is a bad sended packet or not from schueler, dont answer   		op_pk_destroy(pkptr);   		FOUT;   		}   	   #	//a poll request, send queue sizes   +	if (command_type == pkt_poll_poll_request)   		{   		op_pk_destroy(pkptr);   +		pkptr = op_pk_create_fmt("MTA_pkt_poll");   4		op_pk_nfd_set_int32 (pkptr, "src_addr", src_addr);   .		op_pk_nfd_set_int32 (pkptr, "dest_addr", 0);   =		op_pk_nfd_set_int32 (pkptr, "queue_size_1", queue_size[1]);   =		op_pk_nfd_set_int32 (pkptr, "queue_size_2", queue_size[2]);   =		op_pk_nfd_set_int32 (pkptr, "queue_size_3", queue_size[3]);   =		op_pk_nfd_set_int32 (pkptr, "queue_size_4", queue_size[4]);   =		op_pk_nfd_set_int32 (pkptr, "queue_size_5", queue_size[5]);   =		op_pk_nfd_set_int32 (pkptr, "queue_size_6", queue_size[6]);   D		op_pk_nfd_set_int32 (pkptr, "command_type", pkt_poll_poll_answer);   &		op_pk_send (pkptr,MXT_SCH_OUT_STRM);   		}   	   /	//a command, send a packet, update queue sizes   '	if (command_type == pkt_poll_command)    		{		   		   G		op_pk_nfd_get_int32 (pkptr, "queue_dest_addr", &pkt_queue_dest_addr);   M		op_pk_nfd_get_int32 (pkptr, "queue_sending_addr", &pkt_queue_sending_addr);   		op_pk_destroy(pkptr);   		   V		if (queue_size[pkt_queue_dest_addr] == 0) { FOUT; } //I have no data for it to send!   
		flag=-1;   		for (i=0; i<3; i++) {   1			if(mxt_address[i] == pkt_queue_sending_addr) {   			flag = i;   			}   		}   6		if(flag==-1) { FOUT; } //I have no connection to it!   		//send data from queue   		//take out from queue   @		pkptr = op_subq_pk_remove(pkt_queue_dest_addr, OPC_QPOS_TAIL);   $		queue_size[pkt_queue_dest_addr]--;   1		op_pk_nfd_get_int32 (pkptr, "pkt_ID", &pkt_ID);   		   		switch (flag)   			{   ,			case 0:	op_pk_send (pkptr,MXT0_OUT_STRM);   					break;   ,			case 1:	op_pk_send (pkptr,MXT1_OUT_STRM);   					break;   ,			case 2:	op_pk_send (pkptr,MXT2_OUT_STRM);   					break;   			}   		   k		printf("\nPacket Rout: %d--->%d dest:%d(%d)",src_addr,pkt_queue_sending_addr,pkt_queue_dest_addr,pkt_ID);   		}   	   	FOUT;   	}       static void send_hellos()    	{   	Packet* pkptr_temp;	   	FIN(send_hellos())   		   /	pkptr_temp = op_pk_create_fmt("MTA_pkt_data");   8	op_pk_nfd_set_int32 (pkptr_temp, "src_addr", src_addr);   ;	op_pk_nfd_set_int32 (pkptr_temp, "dest_addr", hello_addr);   '	op_pk_send (pkptr_temp,MXT0_OUT_STRM);   	   /	pkptr_temp = op_pk_create_fmt("MTA_pkt_data");   8	op_pk_nfd_set_int32 (pkptr_temp, "src_addr", src_addr);   ;	op_pk_nfd_set_int32 (pkptr_temp, "dest_addr", hello_addr);   '	op_pk_send (pkptr_temp,MXT1_OUT_STRM);   	   /	pkptr_temp = op_pk_create_fmt("MTA_pkt_data");   8	op_pk_nfd_set_int32 (pkptr_temp, "src_addr", src_addr);   ;	op_pk_nfd_set_int32 (pkptr_temp, "dest_addr", hello_addr);   '	op_pk_send (pkptr_temp,MXT2_OUT_STRM);   	   	FOUT;   	}                                         �   �          
   init   
       
   '   2//************************************************   //*******INIT       //get the self address   9op_ima_obj_attr_get(op_id_self(), "Address", &src_addr);        //initialize   hello_addr=999;   for (i=0; i<7; i++) {   	queue_size[i] = 0;   }   for (i=0; i<3; i++) {   	mxt_address[i] = 0;   }       ID=0;       4//send hello packet for mapping mxt to dest addresse   send_hellos();       2//************************************************   //*******FLOW PROGRAMMING       !//detemine which node for sending   flow_sender[0] = 2;   flow_sender[1] = 6;   flow_sender_size = 2;       ,//determine which addresses for sending data   flow_dest[0] = 5;   flow_dest[1] = 4;   flow_dest_size = 2;       2//************************************************   //*******STATISTICS   Iete_gsh = op_stat_reg ("ETE Delay",OPC_STAT_INDEX_NONE,OPC_STAT_GLOBAL);    Rete_hop_count = op_stat_reg ("ETE Hop Count",OPC_STAT_INDEX_NONE,OPC_STAT_GLOBAL);           
                     
   ����   
          pr_state        J   �          
   idle   
                                       ����             pr_state        J  �          
   POLL_RCV   
       
      answer_to_poll();   
                     
   ����   
          pr_state        �  �          
   DATA_RCV   
       
      
in_data();   
                     
   ����   
          pr_state        :   �          
   GEN_RCV   
       
      out_data();   
                     
   ����   
          pr_state                    .   f     N   �  
   V  D   T  O   {  ^   �          
   tr_3   
       
   default   
       ����          
    ����   
          ����                       pr_transition                 P     A   �    O  G  �          
   tr_14   
       
   POLL_RCV   
       ����          
    ����   
          ����                       pr_transition              e  �     U  �  u  N  P   �          
   tr_15   
       ����          ����          
    ����   
          ����                       pr_transition              �  =     R   �  �  H  �  �          
   tr_16   
       
   DATA_RCV   
       ����          
    ����   
          ����                       pr_transition              �  D       ~  �  	  T   �          
   tr_17   
       ����          ����          
    ����   
          ����                       pr_transition              �   �     \   �  �   �  =   �          
   tr_18   
       
   GENERATOR_RCV   
       ����          
    ����   
          ����                       pr_transition                 �     7   �  �   �  U   �          
   tr_19   
       ����          ����          
    ����   
          ����                       pr_transition               �   �      �   �  @   �          
   tr_21   
       ����          ����          
    ����   
          ����                       pr_transition                       	ETE Delay          WCalculates ETE delay by subtracting packet creation time from current simulation time. ������������        ԲI�%��}   ETE Hop Count          %Calculates Hop count for Data Packets������������        ԲI�%��}                                    
   text_0   
       
      Ramyad Hadidi   88109971   OPNET FINAL PROJECT   JUNE 21, 2013   
          ����             ����              ����          
@�     ����   
       
@C      ����   
       
@W�     ����   
       
@H�     ����   
          
annot_text             Annotation Palette          
QĦ�����   
       ����          
@�������   
               ����              ����           ����    