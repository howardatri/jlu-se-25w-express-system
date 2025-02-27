#include<stdio.h>
#include<malloc.h>
typedef struct node{
    int data;            //一定要malloc!
    struct node* next;
}node,*lnode;

lnode create(){
lnode head=NULL,p,q=NULL;
head=(node*)malloc(sizeof(node));
scanf("%d",&head->data);
head->next=NULL;

if(head->data==0)return NULL;
p=head;
while(1){
    int x;
    scanf("%d",&x);
    if(x==0)break;//这里必须要用Q充当空节点
    q=(node*)malloc(sizeof(node));
    q->data=x;
    q->next=NULL;
    p->next=q;//链接
    p=q;//前移
}

return head;
}

lnode delete(lnode head){
    //p当前q下一个 tmp=p辅助p进行覆盖 d进行释放 
   lnode p=NULL;
   p=head;

while(p){
    lnode q,tmp,d;
    tmp=p;//头结点
    q=p->next;
    while(q){
        if(q->data==p->data){
          if(q->next==NULL){
           tmp->next=NULL;
          }
          else{
            tmp->next=q->next;
          }
          d=q;
          q=q->next;
          free(d);
        }
        else{
            tmp=q;//头结点往后移，继续删除
            q=q->next;
        }
    }
    p=p->next;
}
return head;

}

void release(lnode l){
lnode q,p;
p=l;
while(p){
    q=p;
    free(q);
    p=p->next;
}
}
void print(lnode l){
    lnode p;
    p=l;
    while(p->next){
        printf("%d ",p->data);
        p=p->next;
    }
    printf("%d",p->data);
}
int main(){
    lnode head=NULL,res=NULL;
    head=create();
    res=delete(head);
   if(res){
    print(res);
   }else{
    printf("NULL");
   }
   release(head);
    return 0;
}
