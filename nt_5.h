
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <math.h>

#define STRIDE 2

struct MtNode{
	struct MtNode* node[4];  
	int verdict;
};

//typedef struct MtNode node;

struct MtNode* helper(MtNode *curr_node, uint32_t prefix_r, int prelen, int portnum, int b);
uint32_t* paddingFunction(uint32_t prefix_r, int prelen);

/* Initialize multibit trie node */
MtNode* init_mtnode(){
	MtNode *ret = (MtNode *)malloc(sizeof(MtNode));
	if (ret == NULL) 
	    return NULL;
	for (int i = 0 ; i < 4 ; i++){
		ret->node[i] = NULL;
	}
	ret->verdict = -1;
	return ret;
}

/* Clean up binary trie */
void free_mt(struct MtNode *root){
	if(root!=NULL){
		for(int i = 0; i < 4; i++){    
			free_mt(root->node[i]);
		}    
		free(root);	
	}
}

void insert_rule(struct MtNode* root, uint32_t prefix, int prelen, int portnum){

	if( prelen == 0){
		root->verdict = portnum;
	    return;
	}
	if(prelen % 2 == 0){
	    root = helper(root, prefix, prelen, portnum, 0);
	}
	else{ 
	    int padded_prelen = prelen + 1;
	    uint32_t *prefixes;
	    prefixes = paddingFunction(prefix, prelen);
	    for(int i = 0; i < 2; i++){
		    root = helper(root, prefixes[i], padded_prelen, portnum, 0);
	    }
		free(prefixes);
	}       
}

struct MtNode* helper(struct MtNode* curr_node, uint32_t prefix, int prelen, int portnum, int b){
	
	if (b == prelen){
		curr_node->verdict = portnum;
		return curr_node;
	}
	uint32_t  temp_prefix = prefix;
	temp_prefix = prefix << b;
	temp_prefix = temp_prefix & 0xF0000000;
	temp_prefix = temp_prefix >> 30;
	int index = (int) temp_prefix;
	

	if(curr_node->node[index]  == NULL){
		curr_node->node[index] = init_mtnode();
		curr_node = curr_node->node[index];
	}
	else		
		curr_node = curr_node->node[index];
	
	b += 2;
	curr_node = helper(curr_node, prefix, prelen, portnum, b);
	return curr_node;
	
}

uint32_t *paddingFunction(uint32_t prefix_r, int prelen){

	int size = 2;
	uint32_t *arr = (uint32_t *)malloc(size*sizeof(uint32_t));
	//printf("%08x %d \n", prefix_r, prelen);
		
	for(int i = 0; i < 2; i++){
		uint32_t temp = i;
		temp = temp << (31-prelen);
		arr[i] = prefix_r | temp;
		//printf("%08x \n", arr[i]);		
	}
	return arr;
}


/* Function for lookups*/

int lookup(MtNode* root, uint32_t ip){
	
	MtNode *curr_node = root;
	int curr_verdict = root->verdict;
	uint32_t temp_ip = ip;
	int index = 0;
	int b = 0;

	while(1){
		temp_ip = ip << b;
		temp_ip = temp_ip & 0xF0000000;
		temp_ip = temp_ip >> 30;
		index = (int) temp_ip;
		
		if(curr_node->verdict != -1)   
			curr_verdict = curr_node->verdict;
		else
			curr_verdict = curr_verdict;	

		if(curr_node->node[index] == NULL){
			if(curr_node->verdict == -1)   
				return curr_verdict;
			else
				return curr_node->verdict;
		}
		else{
			curr_node = curr_node->node[index];
			b += 2;
		}
		
	}
}

