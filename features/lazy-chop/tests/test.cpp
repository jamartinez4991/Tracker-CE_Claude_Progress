#include <cstdint>
#include <cstdio>
#include <cstring>
#define MAX_SLICE_NUMBER 48
const uint16_t MAX_16BIT=65535;
struct strInstrument{ uint16_t slices[MAX_SLICE_NUMBER]; uint8_t sliceNumber; uint8_t selectedSlice; };
struct mtSliceManager{ uint8_t insertSlicesAtPositions(strInstrument*, const uint16_t*, uint8_t); };
uint8_t mtSliceManager::insertSlicesAtPositions(strInstrument* instr,
                                                const uint16_t* positions,
                                                uint8_t count)
{
    if (instr == nullptr || positions == nullptr || count == 0)
        return 0;
    if (instr->sliceNumber > MAX_SLICE_NUMBER)
        return 0;

    uint16_t sorted[MAX_SLICE_NUMBER];
    if (count > MAX_SLICE_NUMBER)
        count = MAX_SLICE_NUMBER;
    for (uint8_t i = 0; i < count; i++) // insertion sort (<= 48 entries)
    {
        uint16_t v = positions[i];
        int8_t j = (int8_t)i - 1;
        while (j >= 0 && sorted[j] > v)
        {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = v;
    }

    // Work on a copy so the instrument is only touched when something is really inserted.
    uint16_t merged[MAX_SLICE_NUMBER];
    uint8_t n = 0;
    if (instr->sliceNumber == 0)
    {
        merged[n++] = 0; // the zero boundary is not a captured marker
    }
    else
    {
        for (uint8_t i = 0; i < instr->sliceNumber; i++)
            merged[n++] = instr->slices[i];
    }

    uint8_t inserted = 0;
    uint16_t lastInserted = 0;
    for (uint8_t i = 0; i < count; i++)
    {
        const uint16_t p = sorted[i];
        if (p >= MAX_16BIT) // the sample end is not a valid slice start
            continue;
        if (n >= MAX_SLICE_NUMBER)
            break;

        uint8_t pos = 0;
        bool duplicate = false;
        while (pos < n && merged[pos] < p)
            pos++;
        if (pos < n && merged[pos] == p)
            duplicate = true;
        if (duplicate)
            continue;

        for (uint8_t k = n; k > pos; k--)
            merged[k] = merged[k - 1];
        merged[pos] = p;
        n++;
        inserted++;
        lastInserted = p;
    }

    if (inserted == 0)
        return 0;

    for (uint8_t i = 0; i < n; i++)
        instr->slices[i] = merged[i];
    instr->sliceNumber = n;
    for (uint8_t i = 0; i < n; i++)
    {
        if (merged[i] == lastInserted)
        {
            instr->selectedSlice = i;
            break;
        }
    }
    return inserted;
}

static int fails=0;
#define CHECK(c) do{ if(!(c)){ printf("FAIL line %d: %s\n",__LINE__,#c); fails++; } }while(0)
int main(){
  mtSliceManager m; strInstrument s; memset(&s,0,sizeof s);
  uint16_t p[]={30000,10000,10000,50000,0};           // dup, unsorted, 0 duplicates zero boundary
  CHECK(m.insertSlicesAtPositions(&s,p,5)==3);
  CHECK(s.sliceNumber==4 && s.slices[0]==0 && s.slices[1]==10000 && s.slices[2]==30000 && s.slices[3]==50000);
  CHECK(s.selectedSlice==3);
  uint16_t q[]={20000,30000}; s.selectedSlice=1;       // existing dup rejected, mid insert
  CHECK(m.insertSlicesAtPositions(&s,q,2)==1); CHECK(s.sliceNumber==5 && s.slices[2]==20000 && s.selectedSlice==2);
  uint16_t r[]={30000}; strInstrument before=s; s.selectedSlice=4;
  CHECK(m.insertSlicesAtPositions(&s,r,1)==0); CHECK(s.selectedSlice==4 && memcmp(&before.slices,&s.slices,sizeof s.slices)==0);
  strInstrument e; memset(&e,0,sizeof e); uint16_t z[]={0,65535};
  CHECK(m.insertSlicesAtPositions(&e,z,2)==0); CHECK(e.sliceNumber==0);   // nothing inserted: no zero boundary created
  strInstrument f; memset(&f,0,sizeof f); uint16_t many[60]; for(int i=0;i<60;i++) many[i]=(uint16_t)(100+i*10);
  CHECK(m.insertSlicesAtPositions(&f,many,60)==47); CHECK(f.sliceNumber==48);   // 1 zero + 47
  for(int i=1;i<48;i++) CHECK(f.slices[i]>f.slices[i-1]);
  uint16_t one[]={5}; CHECK(m.insertSlicesAtPositions(&f,one,1)==0);
  printf(fails?"%d FAILURES\n":"ALL PASS\n",fails); return fails!=0; }
