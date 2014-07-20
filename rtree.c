#include<stdlib.h>
#include<math.h>
#include<stdio.h>

typedef struct bbox bbox;
struct bbox {
	int min_x;
	int max_x;
	int min_y;
	int max_y;
};
typedef struct node node;
struct node {
	int x;
	int y;
	int childNum;
	node *child[5];
	node *parent;
	bbox box;
	int level;
};
typedef struct queue queue;
struct queue {
	node *entry;
	double dis;
	queue *next;
};
node *rtree;

node* get_leafNode(int x,int y){
	node *new = malloc(sizeof(node));
	new->x = x;
	new->y = y;
	new->level = 0;
	int i;
	for(i = 0 ; i < 5; i++){
		new->child[i] = NULL;
	}
	new->parent = NULL;
	return new;
}

//MBR
node* get_nonleafNode(){
	node *mbr = malloc(sizeof(node));
	mbr->childNum = 0;
	mbr->level = 1;
	mbr->parent = NULL;
	return mbr;
}

int* calBounding(int xy[][2],int index){
	int min_x = 100;
	int max_x = 0;
	int min_y = 100;
	int max_y = 0;
	int x;
	int y;
	int i;
	int *minMax = malloc(4*sizeof(int));
	for(i = 0 ; i < index;i++){
		x = xy[i][0];
		y = xy[i][1];
		if(x<min_x){
			min_x = x;
		}
		if(y<min_y){
			min_y = y;
		}
		if(x>max_x){
			max_x = x;
		}
		if(y>max_y){
			max_y = y;
		}
	}
	minMax[0] = min_x;
	minMax[1] = max_x;
	minMax[2] = min_y;
	minMax[3] = max_y;
	return minMax;
}

int enlargePerimeter(node *mbr,node *entry){
	bbox box = mbr->box;
	int min_x = box.min_x;
	int max_x = box.max_x;
	int min_y = box.min_y;
	int max_y = box.max_y;
	int originPerimeter = (max_x-min_x+max_y-min_y)*2;
	int x = entry->x;
	int y = entry->y;
	if(x<min_x){
		min_x = x;
	}
	if(x>max_x){
		max_x = x;
	}
	if(y<min_y){
		min_y = y;
	}
	if(y>max_y){
		max_y = y;
	}
	int newPerimeter = (max_x-min_x+max_y-min_y)*2;
	return newPerimeter - originPerimeter;
}
node* addToMbr(node *mbr,node *new){
	if(new->level == 0){
		int index = mbr->childNum;
		mbr->child[index] = new;
		new->parent = mbr;
		mbr->childNum = index+1;
		index++;
		int i;
		int min_x;
		int min_y;
		int max_x;
		int max_y;
		int xy[index][2];
		int *minMax;
		for(i = 0;i <index;i++){
			node *n = mbr->child[i];
			xy[i][0] = n->x;
			xy[i][1] = n->y;
		}
		minMax = calBounding(xy,index);
		mbr->box.min_x = minMax[0];
		mbr->box.max_x = minMax[1];
		mbr->box.min_y = minMax[2];
		mbr->box.max_y = minMax[3];
	}else{
		bbox new_bbox = new->box;
		int minx = new_bbox.min_x;
		int maxx = new_bbox.max_x;
		int miny = new_bbox.min_y;
		int maxy = new_bbox.max_y;
		bbox mbr_bbox = mbr->box;
		int mbr_minx = mbr_bbox.min_x;
		int mbr_maxx = mbr_bbox.max_x;
		int mbr_miny = mbr_bbox.min_y;
		int mbr_maxy = mbr_bbox.max_y;
		if(minx < mbr_minx){
			mbr_minx = minx;
		}
		if(maxx > mbr_maxx){
			mbr_maxx = maxx;
		}
		if(miny < mbr_miny){
			mbr_miny = miny;
		}
		if(maxy > mbr_maxy){
			mbr_maxy = maxy;
		}
		mbr_bbox.min_x = mbr_minx;
		mbr_bbox.max_x = mbr_maxx;
		mbr_bbox.min_y = mbr_miny;
		mbr_bbox.max_y = mbr_maxy;
		mbr->box = mbr_bbox;
		int childIndex = mbr->childNum;
		mbr->child[childIndex] = new;
		new->parent = mbr;
		mbr->childNum++;
	}
	return mbr;
}
void BubbleSort(int xy[6][2]){
	int i;
	int j;
	int temp_x;
	int temp_y;
	for(i = 0 ; i <6;i++){
		for(j = 0;j <6-1-i;j++){
			if(xy[j][0]>xy[j+1][0]){
				temp_x = xy[j+1][0];
				temp_y = xy[j+1][1];
				xy[j+1][0] = xy[j][0];
				xy[j+1][1] = xy[j][1];
				xy[j][0] = temp_x;
				xy[j][1] = temp_y;
			}
		}
	}
}

void updateInformation(node *head){
	int itr = head->childNum;
	int i;
	int min_x = 100;
	int max_x = 0;
	int min_y = 100;
	int max_y = 0;
	bbox headBox = head->box;
	for(i = 0;i < itr;i++){
		bbox bounding = head->child[i]->box;
		if(bounding.min_x< min_x){
			min_x = bounding.min_x;	
		}
		if(bounding.max_x > max_x){
			max_x = bounding.max_x;
		}
		if(bounding.min_y < min_y){
			min_y = bounding.min_y;
		}
		if(bounding.max_y > max_y){
			max_y = bounding.max_y;
		}
	}
	headBox.min_x = min_x;
	headBox.max_x = max_x;
	headBox.min_y = min_y;
	headBox.max_y = max_y;
	head->box = headBox;
}

void travelsal(node *head){
	int itr = head->childNum;
	int i;
	if(head->level >=2 ){
		//printf("the %d level\n",head->level);
		//printf("head childNum = %d\n",itr);
		for(i = 0 ; i< itr;i++){
			node *tmp = head->child[i];
			//printf("mbr level : %d\n",tmp->level);
			//printf("%d mbr\n",i);
			//printf("~~~~~~~~~~~\n");
			bbox box = tmp->box;
			//printf("min x : %d\n",box.min_x);
			//printf("max x : %d\n",box.max_x);
			//printf("min y : %d\n",box.min_y);
			//printf("max y : %d\n",box.max_y);
			//printf("~~~~~~~~~~~\n");
			travelsal(tmp);
		}
	}else {
		//printf("the %d level\n",head->level);
		for(i = 0 ; i< itr;i++){
			node *tmp = head->child[i];
			//printf("entry = %d %d\n",tmp->x,tmp->y);
		}
	}
}

void splitAndInsert(node *newMbr,node *nowMbr,node *entry){
	int enlarge1;
	int enlarge2;
	enlarge1 = enlargePerimeter(newMbr,entry);
	enlarge2 = enlargePerimeter(nowMbr,entry);
	if(enlarge1 < enlarge2){
		newMbr = addToMbr(newMbr,entry);
	}else{
		nowMbr = addToMbr(nowMbr,entry);
	}
}

void split_node(node *parent,node *newMbr,node *nowMbr,node *entry){
	if(parent->level >= 3){
		node *pointer = nowMbr;
		node *ptr = newMbr;
		parent = addToMbr(parent,newMbr);
		pointer = pointer->child[0];
		int level = pointer->level;
		while(pointer->level >= 1){
			node *tmp = get_nonleafNode();
			tmp->level = level;
			ptr->childNum++;
			ptr->child[0] = tmp;
			tmp->parent = ptr;
			ptr = ptr->child[0];
			pointer = pointer->child[0];
			level = pointer->level;
		}
	}else{
		//parent's level = 2
		node *index;
		parent = addToMbr(parent,newMbr);
		int i;
		int xy[6][2]; 
		for(i = 0 ; i < 5; i++){
			int x = nowMbr->child[i]->x;
			int y = nowMbr->child[i]->y;
			xy[i][0] = x;
			xy[i][1] = y;
		}
		xy[i][0] = entry->x;
		xy[i][1] = entry->y;
		BubbleSort(xy);
		for(i = 0 ; i < 5 ;i++){
			nowMbr->child[i] =NULL;
		}
		nowMbr->childNum = 0;
		for(i = 0; i< 3;i++){
			node *leaf = get_leafNode(xy[i][0],xy[i][1]);
			nowMbr = addToMbr(nowMbr,leaf);
		}
		for(i = 3;i < 6;i++){
			node *leaf = get_leafNode(xy[i][0],xy[i][1]);
			newMbr = addToMbr(newMbr,leaf);
		}
	}
}

void chooseNodeToInsert(node *parent,node *entry){
	if(parent->level == 1){
		if(parent->childNum == 5){
			node *father = parent->parent;
			node *newNode = get_nonleafNode();
			node *nowMbr = parent;
			split_node(father,newNode,nowMbr,entry);
			//printf("///////////////\n");
		//	travelsal(parent);
			//printf("///////////////\n");
			updateInformation(father);
		}else{
			parent = addToMbr(parent,entry);
			//printf("///////////////\n");
		//	travelsal(parent);
			//printf("///////////////\n");
			node *father = parent->parent;
			updateInformation(father);
		}
	}else{
		if(parent->childNum == 5){
			if(parent->parent==NULL){
			//create new parent and add a newMbr to this parent
				node *newparent = get_nonleafNode();
				newparent = addToMbr(newparent,parent);
				newparent->level = parent->level+1;
				node *newNode = get_nonleafNode();
				newNode->level = parent->level;
				node *nowNode = parent;
				split_node(newparent,newNode,nowNode,entry);
				parent = newparent;
				rtree = parent;
				chooseNodeToInsert(parent,entry);
				updateInformation(parent);
			}
			else{
			//add a newMbr to its parent
				node *father = parent->parent;
				node *newNode = get_nonleafNode();
				newNode->level = parent->level;
				node *nowNode = parent;
				split_node(father,newNode,nowNode,entry);
				updateInformation(father);
			}
		}else{
			int childNum = parent->childNum;
			int betterChoice;
			int nowChoice;
			if(parent->level >= 3){
				betterChoice = childNum-1;
			}
			else{
				if(childNum == 1){
					betterChoice = 0;
				}else{
					int i;
					int j;
					int enlargeNum1;
					int enlargeNum2;
					int minenlarge;
					int enlarge = 10000;
					for(i = 0 ;i <childNum;i++){
						node *mbr1 = parent->child[i];
						enlargeNum1 = enlargePerimeter(mbr1,entry);
						for(j = i+1;j < childNum;j++){
							node *mbr2 = parent->child[j];
							enlargeNum2 = enlargePerimeter(mbr2,entry);
							if(enlargeNum1 <= enlargeNum2){
								nowChoice = i;
								minenlarge = enlargeNum1;
							}
							else{
								nowChoice = j;
								minenlarge = enlargeNum2;
							}
						}
						if(enlarge > minenlarge){
							betterChoice = nowChoice;
							enlarge = minenlarge;
						}
					}
				}
			}
			parent = parent->child[betterChoice];
			chooseNodeToInsert(parent,entry);
		}
	}
}

void createRtree(FILE *ptr,node *parent){
	int data_x;
	int data_y;
	if(!feof(ptr)){
		fscanf(ptr,"%d %d\n",&data_x,&data_y);
		//printf("level = %d\n",parent->level);
		//printf("%d %d\n",data_x,data_y);
		node *entry = get_leafNode(data_x,data_y);
		chooseNodeToInsert(parent,entry);
		//printf("after insert node , rtree level = %d\n",rtree->level);
		//printf("~~rtree childnum = %d\n",rtree->childNum);
		parent = rtree;
		createRtree(ptr,parent);
	}else{
		//printf("!!!!!!!!!!\n");
		travelsal(rtree);
		/*printf("!!!!!!!!!!\n");
		printf("**********\n");
		printf("rtree head:\n");
		printf("min x : %d\n",rtree->box.min_x);
		printf("max x : %d\n",rtree->box.max_x);
		printf("min y : %d\n",rtree->box.min_y);
		printf("max y : %d\n",rtree->box.max_y);
		printf("**********\n");*/
	}
}
queue *getQueueNode(){
	queue *que = malloc(sizeof(queue));
	return que;
}
int dominate(node *entry1,node *entry2){
	int x1;
	int x2;
	int y1;
	int y2;
	x1 = entry1->x;
	y1 = entry1->y;
	x2 = entry2->x;
	y2 = entry2->y;
	if((x1 < x2 && y1 < y2) ||(x1 == x2 && y1 < y2 )||(x1 < x2 && y1 == y2)){
		return 0;
	}else if((x1 < x2 && y1 > y2) || (x1 == x2 && y1 ==y2) || (x1 > x2 && y1 < y2)){
		return 1;
	}else{
		return 2;
	}
}

double calDis(node *entry){
	double dis;
	if(entry->level == 0){
		double x = entry->x;
		double y = entry->y;
		dis = x*x+y*y;
		dis = sqrt(dis);
	}else{
		bbox box = entry->box;
		double minx = box.min_x;
		double miny = box.min_y;
		dis = minx*minx+miny*miny;
		dis = sqrt(dis);
	}
	return dis;
}

queue *addQueue(queue *head,node *entry,double dis){
	//printf("add queue!\n");
	if(head == NULL){
		//printf("head null!!\n");
		queue *new = getQueueNode();
		head = new;
		new->entry = entry;
		new->dis = dis;
		new->next = NULL;
	}else{
		//printf("yaya dis =%f\n",dis);
		queue *pointer = head;
		while(pointer->next != NULL){
			pointer = pointer->next;
		}
		queue *new = getQueueNode();
		pointer->next = new;
		new->entry = entry;
		new->dis = dis;
		new->next=NULL;
	}
	return head;
}

queue* deQueue(queue *head){
	//printf("deQueue!!\n");
	queue *top = head;
	return top;
}

void swap(queue *entry1,queue *entry2){
	node *tmp = entry1->entry;
	double distmp = entry1->dis;
	entry1->entry = entry2->entry;
	entry1->dis = entry2->dis;
	entry2->entry = tmp;
	entry2->dis = distmp;
}

void *sort(queue *que){
	//printf("sort!!\n");
	int swapped;
	int i;
	queue *ptr1 = que;
	queue *lptr = NULL;
	if(ptr1 == NULL){
		return;
	}
	do {
		swapped = 0;
		ptr1 = que;

		while(ptr1->next != lptr){
			if(ptr1->dis > ptr1->next->dis){
				swap(ptr1,ptr1->next);
				swapped = 1;
			}
			ptr1 = ptr1->next;
		}
		lptr =ptr1;
	}while(swapped);
	//printf("sort end!!\n");
}

void printQueue(queue *skyline){
	node *entry;
	while(skyline != NULL){
		entry = skyline->entry;
		printf("%d %d\n",entry->x,entry->y);
		skyline = skyline->next;
	}
}




void bbs(queue *queHead,node *head){
	node *entry1;
	node *entry2;
	queue *queItr;
	queItr = queHead;
	queue *skyline = NULL;
	queue *pskyPtr = NULL;
	int itr = head->childNum;
	int i;
	double dis;
	for(i = 0; i < itr;i++){
		node *tmp = head->child[i];
		dis = calDis(tmp);
		queHead = addQueue(queHead,tmp,dis);
	}
	sort(queHead);
	while(queHead != NULL){
		queue *top = deQueue(queHead);
		queHead = queHead->next;
		if(top->entry->level == 0){
			if(skyline == NULL){
				dis = calDis(top->entry);
				skyline = addQueue(skyline,top->entry,dis);
			}else{
				queue *skyPtr = skyline;
				while(skyPtr!=NULL){
					entry1 = top->entry;
					entry2 = skyPtr->entry;
					int domin = dominate(entry1,entry2);
					if(domin == 0){
						if(skyPtr == skyline){
							//start
							skyline = skyline->next;
							pskyPtr = skyPtr;
							skyPtr = skyPtr->next;
						}else if(skyPtr->next == NULL){
							//tail
							pskyPtr->next = NULL;
						}else{
							pskyPtr->next = skyPtr->next;
							skyPtr = skyPtr->next;
						}				
					}else if(domin ==1){
						pskyPtr = skyPtr;
						skyPtr = skyPtr->next;
					}else{
						top = NULL;
						break;
					}
				}
				if(top!=NULL){
					dis = calDis(top->entry);
					skyline = addQueue(skyline,top->entry,dis);
				}
			}
		}else{
			int childNum = top->entry->childNum;
			for(i = 0 ; i < childNum;i++){
				node *tmp = top->entry->child[i];
				dis = calDis(tmp);
				addQueue(queHead,tmp,dis);
			}
			sort(queHead);
		}
		free(top);
	}
	printf("~~~~~~~~~~~~\n");
	printf("bbs result~\n");
	printQueue(skyline);
}


int main(int argc,char *argv[]){
	FILE *ptr;
	if(argc < 2){
		printf("please choose input file\n");
		return 0;
	}else{
		ptr = fopen(argv[1],"r");
	}
	if(ptr==NULL){
		printf("fuck\n");
	}
	//initial rtree
	rtree = get_nonleafNode();
	node *mbr = get_nonleafNode();
	rtree->child[0] = mbr;
	rtree->level = 2;
	mbr->parent = rtree;
	node *head = rtree;
	rtree->childNum = 1;
	createRtree(ptr,head);
	queue *priorityQueue;
	printf("creating rtree...\n");
	printf("head level = %d\n",rtree->level);
	printf("rtree is created!!\n");
	printf("start bbs\n");
	printf("loading...\n");
	bbs(priorityQueue,rtree);
	printf("bbs is done!");
	return 0;
}
