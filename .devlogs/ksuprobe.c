typedef unsigned int u32; typedef unsigned long u64;
#define _IOC(d,t,nr,sz) (((d)<<30)|(((sz)&0x3fff)<<16)|(((t)&0xff)<<8)|((nr)&0xff))
struct st { u32 manager_appid, valid, pfs, obs, crown, bc, mm, allow, deny; };
struct ht { char hook_type[32]; };
#define CMD_DEBUG _IOC(2,'K',99,36)
#define CMD_HOOK  _IOC(2,'K',101,0)
#define MAGIC1 0xDEADBEEF
#define MAGIC2 0xCAFEBABE
static long sys4(long n,long a,long b,long c,long d){
    register long x8 __asm__("x8")=n; register long x0 __asm__("x0")=a;
    register long x1 __asm__("x1")=b; register long x2 __asm__("x2")=c; register long x3 __asm__("x3")=d;
    __asm__ volatile("svc #0":"+r"(x0):"r"(x8),"r"(x1),"r"(x2),"r"(x3):"memory"); return x0; }
static long sys3(long n,long a,long b,long c){
    register long x8 __asm__("x8")=n; register long x0 __asm__("x0")=a;
    register long x1 __asm__("x1")=b; register long x2 __asm__("x2")=c;
    __asm__ volatile("svc #0":"+r"(x0):"r"(x8),"r"(x1),"r"(x2):"memory"); return x0; }
#define SYS_reboot 142
#define SYS_ioctl 29
#define SYS_write 64
#define SYS_exit 93
static void puts_(const char*s){long n=0;while(s[n])n++;sys3(SYS_write,1,(long)s,n);}
static void putu(u32 v){char b[12];int i=11;b[i]=0;if(!v)b[--i]='0';while(v){b[--i]=(char)('0'+v%10);v/=10;}puts_(&b[i]);}
static void kv(const char*k,u32 v){puts_(k);puts_("=");putu(v);puts_("\n");}
void _start(void){
    int fd=-1; sys4(SYS_reboot,(long)(int)MAGIC1,(long)(int)MAGIC2,0,(long)&fd);
    puts_("fd="); putu((u32)fd); puts_("\n");
    static struct st s;
    long r=sys3(SYS_ioctl,fd,CMD_DEBUG,(long)&s);
    puts_("DEBUG_STATE ret="); puts_(r<0?"-":""); putu((u32)(r<0?-r:r)); puts_("\n");
    kv("manager_appid", s.manager_appid);
    kv("manager_appid_valid", s.valid);
    kv("post_fs_data", s.pfs);
    kv("observer", s.obs);
    kv("crown", s.crown);
    kv("boot_completed", s.bc);
    kv("module_mounted", s.mm);
    kv("allow_list", s.allow);
    kv("deny_list", s.deny);
    static struct ht h;
    r=sys3(SYS_ioctl,fd,CMD_HOOK,(long)&h);
    puts_("GET_HOOK_TYPE ret="); puts_(r<0?"-":""); putu((u32)(r<0?-r:r)); puts_(" type='"); puts_(h.hook_type); puts_("'\n");
    sys3(SYS_exit,0,0,0);
}
