#include<iostream>
#include<fstream>
#include<vector>
#include<queue>
#include<ctime>
using namespace std;
struct huffman
{
    bool isLeaf;
    unsigned char content;
    int weight;
    huffman *l,*r;
    huffman()
    {
        isLeaf=1;
        content=0;
        weight=0;
        l=r=nullptr;
    }
    huffman(char x,int y)
    {
        isLeaf=1;
        content=(unsigned char)x;
        weight=y;
        l=r=nullptr;
    }
    huffman(huffman *lp,huffman *rp)
    {
        isLeaf=0;
        weight=lp->weight+rp->weight;
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
unsigned long long fileSize=0;
time_t ta, tb;
int gf;
void help()
{
    cout<<"A Huffman coding and decoding program."<<endl;
    cout<<"Usage: "<<endl;
    cout<<"huffman c <in> <out> - Code"<<endl;
    cout<<"huffman d <in> <out> - Decode"<<endl;
    cout<<"Use at your own risk!"<<endl;
    cout<<"Author: Vicky Silviana"<<endl;
}
void printProgress(int val)
{
    cout << "\r[";
    for (int i = 0; i < val/5; i++)cout << "#";
    for (int i = val/5; i < 20; i++)cout << ".";
    cout << "](" << val << "%)";
    fflush(stdout);
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
        vec[cur->content]=stk;
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
bool feed(queue<bool>& q)
{
    char c;
    if (!ifs.get(c))return false;
    for(int i=7;i>=0;i--)q.push((c>>i)%2);
    return true;
}
void coding()
{
    unsigned long long cnt[256]={0};
    char tmpc;
    huffman *lcur,*rcur;
    vector<bool> vec[256];
    vector<bool> stk;
    queue<bool> oq;
    time(&ta);
    unsigned long long val=0,curSize=0,finalSize=0;
    cout << "Preflight...\n";
    while(ifs.get(tmpc))cnt[(unsigned char)tmpc]++;
    priority_queue<huffman*,vector<huffman*>,cmp> pq;
    /* Build Huffman tree. */
    for(int i=0;i<256;i++)if(cnt[i]!=0)pq.push(new huffman(i,cnt[i]));
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
    fileSize = val;
    ifs.clear();
    ifs.seekg(0,ios::beg);
    val=0;
    /* Write data. */
    printProgress(0);
    while(ifs.get(tmpc))
    {
        curSize++;
        if (curSize * 100 / fileSize != (curSize - 1) * 100 / fileSize)printProgress(curSize*100/fileSize);
        for(int i=0;i<vec[(unsigned char)tmpc].size();i++)oq.push(vec[(unsigned char)tmpc][i]);
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
            finalSize++;
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
    finalSize++;
    printProgress(100);
    time(&tb);
    cout << "\nOperation done in "<<tb-ta<<"s with compression rate " << finalSize*100 / fileSize << "%";
}
void decoding()
{
    char head[3],tmpc,tmpa,tmpb;
    huffman *node[512],*cur;
    int fa;
    queue<bool> q;
    time(&ta);
    /* Read file header. */
    for(int i=0;i<3;i++)ifs.get(head[i]);
    if(head[0]!='V'||head[1]!='S'||head[2]!='Z')
    {
        cout << "FATAL: Invalid header!";
        return;
    }
    for(int i=0;i<511;i++)
    {
        ifs.get(tmpc);
        ifs.get(tmpa);
        ifs.get(tmpb);
        node[i]=new huffman(tmpc,0);
        if(tmpa>1)continue;
        fa=(int)tmpa<<8|(unsigned char)tmpb;
        if((fa>512&&fa!=1024)||!node[fa])
        {
            cout << "FATAL: Invalid tree structure!";
            return;
        }
        if(node[fa]->isLeaf)
        {
            node[fa]->isLeaf=0;
            node[fa]->l=node[i];
        }
        else node[fa]->r=node[i];
    }
    cur=node[0];
    fileSize = 0;
    for(int i=0;i<8;i++)
    {
        ifs.get(tmpc);
        fileSize<<=8;
        fileSize+=(tmpc>=0?tmpc:tmpc+256);
    }
    /* Restore file. */
    printProgress(0);
    for(unsigned long long i=0;i<fileSize;i++)
    {
        if (i * 100 / fileSize != (i - 1) * 100 / fileSize)printProgress(i * 100 / fileSize);
        while (!cur->isLeaf)
        {
            if (q.empty())
            {
                if(!feed(q))
                {
                    cout << "\nFATAL: EOF before correct position!("<<i<<" vs "<<fileSize<<")\nFile damaged.";
                    return;
                }
            }
            cur=q.front()?cur->r:cur->l;
            q.pop();
        }
        ofs<<(char)cur->content;
        cur=node[0];
    }
    printProgress(100);
    time(&tb);
    cout << "\nOperation done in " << tb - ta << "s";
}
int main(int argc, char** argv)
{
    if(argc!=4||(argv[1][0]!='c'&&argv[1][0]!='d'))
    {
        help();
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
