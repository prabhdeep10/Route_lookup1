#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

//#define DEBUG

/* Structure of binary trie node */
struct BtNode{
    BtNode  *left;      /* for 0 */
    BtNode  *right;     /* for 1 */
    int verdict;    // portnumber
	int skip; 			// skipped node 
	uint32_t segment; 			// skipped segment
	
};

/* Initialize binary trie node */
BtNode* init_btnode(){
    BtNode *ret = (BtNode *)malloc(sizeof(BtNode));
    ret->left = NULL;
    ret->right = NULL;
    ret->verdict = -1;
    ret->skip = 0;
    ret->segment = 0;
    return ret;
}

/* Clean up binary trie */
void free_bt(BtNode *root){

    if(root->left != NULL){
        free_bt(root->left);
    }
    if(root->right != NULL){
        free_bt(root->right);
    }

    free(root);
}

/* Insert a rule */
void insert_rule(BtNode *root, uint32_t prefix, int prelen, int portnum){
    static int     n_rules = 0;

#ifdef DEBUG
    uint32_t prefix_r = htonl(prefix);
    fprintf(stderr, "Insert rule: %-15s(%08x)/%d    %d\n", 
            inet_ntoa(*(struct in_addr *)&prefix_r), 
            prefix, prelen, portnum);
#endif

    n_rules ++;

    /* default rule: if packet matches none of the rules, 
     * it will match this default rule, i.e. 0.0.0.0/0 */
    if( prelen == 0 ){
        root->verdict = portnum;
        return;
    }

    uint32_t    temp_prefix = prefix;
    BtNode      *curr_node = root;
    for(int i=0 ; i<prelen ; i++){
        int     curr_bit = (temp_prefix & 0x80000000) ? 1 : 0;
        if(curr_bit == 0){
            if(curr_node->left == NULL){
                curr_node->left = init_btnode();
            }
            curr_node = curr_node->left;
        }
        else{
            if(curr_node->right == NULL){
                curr_node->right = init_btnode();
            }
            curr_node = curr_node->right;
        }
        temp_prefix = temp_prefix << 1;
    }

    if( curr_node->verdict != -1 ){
        fprintf(stderr, "Error: Rule #%d - overwriting a previous rule!! \n", n_rules);
    }
    curr_node->verdict = portnum;
}


void compress_tree(BtNode *curr_node, BtNode *parent){
	// Insert Compressing node rules here
int x;//x is used to define the skip value of each node//
uint32_t y;//y is used to define the segment value of each node//

	if (curr_node!=NULL){
		// If parent is null then it means that the current reference is the  curr_node Node
		
		  if(curr_node->verdict==-1){ //If verdict is -1 means it is an intermediate node and doesn't contain any prefix and can be compressed//
		  		if(parent!=NULL){//parent = NULL means it is a root node and obviously can't be compressed//
					if(parent->right == curr_node){//if the node under consideration is on the right side of the parent//
						if(curr_node->left == NULL  &&  curr_node->right != NULL){//the basic condition for the node to be compressed is if it 																						points to a single child//
                    x = 0;
                    uint32_t y = 0;
                    parent->right = curr_node->right;//parent will then point to the child of the node to be compressed//
                    x = curr_node->skip;
                    curr_node->right->skip = x + 1;//incrementing the skip value of the node//
                    
                    y = curr_node->segment;
                    y = y << 1;
                    curr_node->right->segment = y | 0x00000001;//since the current node is pointing to a "1", then the segment value will be OR'ed 																	with 0x1// 
                    
					free(curr_node);
					curr_node = parent;
				}
				else if(curr_node->right == NULL  &&  curr_node->left != NULL){
                    x = 0;
                    uint32_t y = 0;
                    parent->right = curr_node->left;
                    
                    x = curr_node->skip;
                    curr_node->left->skip = x + 1;
                    
                    y = curr_node->segment;
                    y = y << 1;
                    curr_node->left->segment = y | 0x00000000;
                    
					free(curr_node);
					curr_node = parent;
				}
              }
                
				
			
            
			else{
				if(curr_node->left == NULL  &&  curr_node->right != NULL){//if the node under consideration is on the left side of the parent//
                    x = 0;
                    uint32_t y = 0;
                    parent->left = curr_node->right;
                    
                    x = curr_node->skip;
                    curr_node->right->skip = x + 1;
                    
                    y = curr_node->segment;
                    y = y << 1;
                    curr_node->right->segment = y | 0x00000001;
                    
                    free(curr_node);
					curr_node = parent;
                    }
				else if(curr_node->right == NULL  &&  curr_node->left != NULL){
                    x = 0;
                    uint32_t y = 0;
                    parent->left = curr_node->left;
                    
                    x = curr_node->skip;
                    curr_node->left->skip  = x + 1;
                    
                    y = curr_node->segment;
                    y = y << 1;
                    curr_node->left->segment = y | 0x00000000;
                    
                    
					free(curr_node);
					curr_node = parent;
                    }
				// If both are null thne it means it is a leaf node and it doesn't have to be compressed
			 }
			}
		}
		
		compress_tree(curr_node->left, curr_node);//using recursion//
		compress_tree(curr_node->right, curr_node);
		}
	}
	
	
	 

//////////////////////////////////////////////////////

/* Look up an IP address (represented in a uint32_t)*/
int lookup_ip(BtNode *root, uint32_t ip){
    uint32_t    temp_ip = ip;
    BtNode      *curr_node = root;
    BtNode      *temp_node = root;
    int         curr_verdict = root->verdict;
    int         curr_bit = 0;
    int         temp_verdict;
    int i;
    int j;
    uint32_t    skip_ip = ip;
    int temp_bit;
    int seg_bit;
    int a[35];
    int b[35];
    
    int c[35];
    int d[35];
    
    

    while(1){
        curr_bit = (temp_ip & 0x80000000) ? 1 : 0;
        skip_ip = temp_ip;
	if(curr_node->skip == 0){//if the node's skip value is 0, then the opeation is the same as the binary trie//
		if(curr_bit == 0){
			if(curr_node->left == NULL) return curr_verdict;
		  	else 			    curr_node = curr_node->left;
		}
		else{
			if(curr_node->right == NULL) return curr_verdict;
			else			     curr_node = curr_node->right;
		}
      }
	else{//skip value non zero//
	
                        int z = 0;
						z = curr_node->skip;
						uint32_t curr_node_segment = curr_node->segment;//storing the node's segment value in the variable//
                        for(i = 0;i<z;i++){//loop will go on for the skip number of time//
                            
                            temp_bit = (skip_ip & 0x80000000) ? 1 : 0;//extracting the bits in the ip address corresponding to the skip value and 																			storing them in an array//
			    			skip_ip = skip_ip << 1;
                            a[i] = temp_bit;
                            seg_bit = (curr_node_segment & 0x00000001) ? 1 : 0;//extracting the node's segment and storing them in  an array//
                            b[i] = seg_bit;
                            curr_node_segment = curr_node_segment >> 1;
                            
                        }
                        for(j = 0;j<z;j++){
                            if(a[j] != b[z-1-j])//comparing the ip address bits with the segment bits and if they don't match, just return the 														verdict of the previous node traversed stored in the form of temp verdict//
                                return temp_verdict;
                                
                        }
		temp_ip = temp_ip << z;//if the bits match, then check the bit in the ip address after the skip number of bits to find out the next node//
		curr_bit = (temp_ip & 0x80000000) ? 1 : 0; 
		if(curr_bit == 0){
			if(curr_node->left == NULL) return curr_verdict;
		  	else 			    curr_node = curr_node->left;
			}
		else{
			if(curr_node->right == NULL) return curr_verdict;
			else			     curr_node = curr_node->right;
		}
		
                    }
	temp_verdict = curr_verdict;
         //update verdict if current node has an non-empty verdict ???????? 
        curr_verdict = (curr_node->verdict == -1) ? curr_verdict : curr_node->verdict;
        temp_ip = temp_ip << 1;
            
            }
	}

       

