#include<iostream>
#include<fstream>
#include<vector>
#include<queue>
using namespace std;
struct huffman
{
    bool isLeaf;
    char content;
    int weight;
    int size;
    huffman *l,*r;
    huffman()
    {
        isLeaf=1;
        content=0;
        weight=0;
        size=1;
        l=r=nullptr;
    }
    huffman(char x,int y)
    {
        isLeaf=1;
        content=x;
        weight=y;
        size=1;
        l=r=nullptr;
    }
    huffman(huffman *lp,huffman *rp)
    {
        isLeaf=0;
        weight=lp->weight+rp->weight;
        size=lp->size+rp->size;
        l=lp;
        r=rp;
    }
};
/* For priority queue. */
struct cmp
{
    bool operator () (huffman* a,huffman* b)
    {
        return a->weight>b->weight;
    }
};
ifstream ifs;
ofstream ofs;
int gf;
void help(char* myName)
{
    cout<<"A Huffman coding and decoding program."<<endl;
    cout<<"Usage: "<<endl;
    cout<<myName<<" c <in> <out> - Code"<<endl;
    cout<<myName<<" d <in> <out> - Decode"<<endl;
    cout<<"Use at your own risk!"<<endl;
    cout<<"Author: Vicky Silviana"<<endl;
}
/* Recursively travel through the tree.
 *
 * Generate char->bits map.
 * Write file header at the same time.
 *
 */
void travel(huffman* cur,vector<bool>* vec,vector<bool>& stk,int fa)
{
    int curid=gf++;
    if(cur->isLeaf)
    {
        vec[cur->content+128]=stk;
        ofs<<cur->content<<(char)(fa/256)<<(char)(fa%256);
        return;
    }
    ofs<<'\0'<<(char)(fa/256)<<(char)(fa%256);
    stk.push_back(0);
    travel(cur->l,vec,stk,curid);
    stk.pop_back();
    stk.push_back(1);
    travel(cur->r,vec,stk,curid);
    stk.pop_back();
}
/* Get more bits from ifs. */
void feed(queue<bool>& q)
{
    char c;
    ifs.get(c);
    for(int i=7;i>=0;i--)q.push((c>>i)%2);
}
void coding()
{
    long long cnt[256]={0};
    char tmpc;
    huffman *lcur,*rcur;
    vector<bool> vec[256];
    vector<bool> stk;
    queue<bool> oq;
    long long val=0;
    while(ifs.get(tmpc))cnt[tmpc+128]++;
    priority_queue<huffman*,vector<huffman*>,cmp> pq;
    /* Build Huffman tree. */
    for(int i=0;i<256;i++)if(cnt[i]!=0)pq.push(new huffman(i-128,cnt[i]));
    while(pq.size()>1)
    {
        lcur=pq.top();
        pq.pop();
        rcur=pq.top();
        pq.pop();
        pq.push(new huffman(lcur,rcur));
    }
    huffman* root=pq.top();
    gf=0;
    /* Write file header. */
    ofs<<"VSZ";
    travel(root,vec,stk,512);
    for(int i=0;i<256;i++)val+=(cnt[i]!=0);
    for(int i=0;i<512-2*val;i++)ofs<<'\0'<<'\004'<<'\0';
    val=0;
    for(int i=0;i<256;i++)val+=cnt[i];
    for(int i=7;i>=0;i--)ofs<<(char)(val>>(i*8)%256);
    ifs.clear();
    ifs.seekg(0,ios::beg);
    val=0;
    /* Write data. */
    while(ifs.get(tmpc))
    {
        for(int i=0;i<vec[tmpc+128].size();i++)oq.push(vec[tmpc+128][i]);
        while(oq.size()>=8)
        {
            val=0;
            for(int i=0;i<8;i++)
            {
                val*=2;
                val+=oq.front();
                oq.pop();
            }
            ofs<<(char)val;
        }
    }
    /* Force write a byte to store data at the end. */
    val=oq.size();
    for(int i=0;i<8-val;i++)oq.push(0);
    val=0;
    for(int i=0;i<8;i++)
    {
        val*=2;
        val+=oq.front();
        oq.pop();
    }
    ofs<<(char)val;
}
void decoding()
{
    char head[3],tmpc,ta,tb;
    huffman *node[512],*cur;
    long long fsize;
    int fa;
    queue<bool> q;
    /* Read file header. */
    for(int i=0;i<3;i++)ifs.get(head[i]);
    for(int i=0;i<511;i++)
    {
        ifs.get(tmpc);
        ifs.get(ta);
        ifs.get(tb);
        node[i]=new huffman(tmpc,0);
        if(ta>1)continue;
        fa=(ta>=0?ta:ta+256)*256+(tb>=0?tb:tb+256);
        if(node[fa]->isLeaf)
        {
            node[fa]->isLeaf=0;
            node[fa]->l=node[i];
        }
        else node[fa]->r=node[i];
    }
    cur=node[0];
    fsize = 8;
    for(int i=0;i<8;i++)
    {
        ifs.get(tmpc);
        fsize<<=8;
        fsize+=(tmpc>=0?tmpc:tmpc+256);
    }
    /* Restore file. */
    for(long long i=0;i<fsize;i++)
    {
        while(!cur->isLeaf)
        {
            if(q.empty())feed(q);
            cur=q.front()?cur->r:cur->l;
            q.pop();
        }
        ofs<<cur->content;
        cur=node[0];
    }
}
int main(int argc, char** argv)
{
    if(argc!=4||(argv[1][0]!='c'&&argv[1][0]!='d'))
    {
        help(argv[0]);
        return 0;
    }
    ifs.open(argv[2],std::ifstream::in|std::ifstream::binary);
    ofs.open(argv[3],std::ofstream::out|std::ofstream::binary);
    if(argv[1][0]=='c')
    {
        coding();
    }
    if(argv[1][0]=='d')
    {
        decoding();
    }
    return 0;
}
