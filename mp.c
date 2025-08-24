// Multiprocessor support
// Search memory for MP description structures.
// http://developer.intel.com/design/pentium/datashts/24201606.pdf

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mp.h"
#include "x86.h"
#include "mmu.h"
#include "proc.h"

struct cpu cpus[NCPU];
int ncpu;
uchar ioapicid;

static uchar
sum(uchar *addr, int len)
{
  int i, sum;

  sum = 0;
  for(i=0; i<len; i++)
    sum += addr[i];
  return sum;
}

// Look for an MP structure in the len bytes at addr.
static struct mp*
mpsearch1(uint a, int len)
{
  uchar *e, *p, *addr;

  addr = P2V(a);
  e = addr+len;
  for(p = addr; p < e; p += sizeof(struct mp))
    if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
      return (struct mp*)p;
  return 0;
}

// Search for the MP Floating Pointer Structure, which according to the
// spec is in one of the following three locations:
// 1) in the first KB of the EBDA;
// 2) in the last KB of system base memory;
// 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
static struct mp*
mpsearch(void)
{
  uchar *bda;
  uint p;
  struct mp *mp;

  bda = (uchar *) P2V(0x400);
  if((p = ((bda[0x0F]<<8)| bda[0x0E]) << 4)){
    if((mp = mpsearch1(p, 1024)))
      return mp;
  } else {
    p = ((bda[0x14]<<8)|bda[0x13])*1024;
    if((mp = mpsearch1(p-1024, 1024)))
      return mp;
  }
  return mpsearch1(0xF0000, 0x10000);
}

// Search for an MP configuration table.  For now,
// don't accept the default configurations (physaddr == 0).
// Check for correct signature, calculate the checksum and,
// if correct, check the version.
// To do: check extended table checksum.
static struct mpconf*
mpconfig(struct mp **pmp)
{
  struct mpconf *conf;
  struct mp *mp;

  if((mp = mpsearch()) == 0)
    return 0;
  if(mp->physaddr == 0)
    return 0;
    
  uint physaddr = (uint)mp->physaddr;
  conf = (struct mpconf*)P2V(physaddr);
  
  if(conf == 0)
    return 0;
    
  // Check signature before accessing other fields
  if(memcmp(conf, "PCMP", 4) != 0)
    return 0;
    
  // Copy values to local variables to avoid array bounds errors
  uchar version = conf->version;
  if(version != 1 && version != 4)
    return 0;
    
  // Use a local variable for length to avoid array bounds warning
  ushort len = conf->length;
  if(len < sizeof(struct mpconf))
    return 0;
    
  // Validate checksum with safer approach
  uchar *addr = (uchar*)conf;
  int checksum = 0;
  for(int i = 0; i < len && i < 4096; i++) { // Add max length check
    checksum += addr[i];
  }
  
  if((checksum & 0xFF) != 0)
    return 0;
    
  *pmp = mp;
  return conf;
}

void
mpinit(void)
{
  uchar *p, *e;
  int ismp;
  struct mp *mp = 0;
  struct mpconf *conf = 0;
  struct mpproc *proc;
  struct mpioapic *ioapic;

  conf = mpconfig(&mp);
  if(conf == 0)
    panic("Expect to run on an SMP");
    
  ismp = 1;
  
  // Safe copy of the lapicaddr pointer
  uint *lapic_addr = 0;
  if(conf)
    lapic_addr = (uint*)conf->lapicaddr;
  lapic = lapic_addr;
  
  // Safe copy of the length
  ushort conf_len = 0;
  if(conf)
    conf_len = conf->length;
    
  // Calculate end pointer safely
  p = (uchar*)(conf+1);
  e = (uchar*)conf + conf_len;
  
  // Only proceed if pointers are valid
  if(p && e && p < e) {
    while(p < e) {
      switch(*p) {
      case MPPROC:
        proc = (struct mpproc*)p;
        if(ncpu < NCPU) {
          cpus[ncpu].apicid = proc->apicid;  // apicid may differ from ncpu
          ncpu++;
        }
        p += sizeof(struct mpproc);
        break;
      case MPIOAPIC:
        ioapic = (struct mpioapic*)p;
        ioapicid = ioapic->apicno;
        p += sizeof(struct mpioapic);
        break;
      case MPBUS:
      case MPIOINTR:
      case MPLINTR:
        p += 8;
        break;
      default:
        ismp = 0;
        p = e;  // Exit the loop
        break;
      }
      
      // Safety check to prevent infinite loop
      if(p > e) {
        ismp = 0;
        break;
      }
    }
  }
  
  if(!ismp)
    panic("Didn't find a suitable machine");

  if(mp && mp->imcrp){
    // Bochs doesn't support IMCR, so this doesn't run on Bochs.
    // But it would on real hardware.
    outb(0x22, 0x70);   // Select IMCR
    outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
  }
}
