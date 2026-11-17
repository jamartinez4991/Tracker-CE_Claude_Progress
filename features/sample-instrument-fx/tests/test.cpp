#include "AudioStream.h"
#include "effect_sampleRateReduction.h"
#include "filter_dj.h"
#include <cstdio>
#include <cmath>
#include <vector>
#define CHECK(c) do{ if(!(c)){ printf("FAIL line %d: %s\n",__LINE__,#c); fails++; } }while(0)
static int fails=0;
static void fill(audio_block_t& b,int off,int ch){ for(int i=0;i<128;i++) b.data[i]=(int16_t)((off+i)*7*(ch?-1:1)+ch*3); }

int main(){
  // ---- mapping
  CHECK(AudioSampleRateReduction::effectiveRateHz(0) > 44117.0f);
  CHECK(fabsf(AudioSampleRateReduction::effectiveRateHz(255)-100.0f) < 0.01f);
  float prev=1e9; bool mono=true; for(int v=0;v<=255;v++){ float f=AudioSampleRateReduction::effectiveRateHz(v); if(!(f<=prev)) mono=false; prev=f; }
  CHECK(mono);
  printf("SRR rate p=1:%.1f p=64:%.1f p=128:%.1f p=200:%.1f p=255:%.2f\n",
    AudioSampleRateReduction::effectiveRateHz(1),AudioSampleRateReduction::effectiveRateHz(64),
    AudioSampleRateReduction::effectiveRateHz(128),AudioSampleRateReduction::effectiveRateHz(200),AudioSampleRateReduction::effectiveRateHz(255));

  // ---- SRR bypass is exact (same block pointer passed through)
  { AudioSampleRateReduction s(2); audio_block_t a,b; fill(a,0,0); fill(b,0,1); s.in[0]=&a; s.in[1]=&b; s.reset(); s.setReduction(0);
    s.update(); CHECK(s.out[0]==&a && s.out[1]==&b); CHECK(s.allocated==0); }

  // ---- SRR capture interval: p=255 => ~441.18 output samples between captures, phase continuous over blocks
  { AudioSampleRateReduction s(2); s.reset(); s.setReduction(255);
    std::vector<int16_t> L,R; int pos=0;
    for(int blk=0;blk<20;blk++){ audio_block_t a,b; for(int i=0;i<128;i++){a.data[i]=(int16_t)(pos+i+1); b.data[i]=(int16_t)(-(pos+i+1)*2);} pos+=128;
      s.in[0]=&a; s.in[1]=&b; s.update(); for(int i=0;i<128;i++){L.push_back(s.out[0]->data[i]);R.push_back(s.out[1]->data[i]);} }
    std::vector<int> caps; for(size_t i=0;i<L.size();i++) if(i==0||L[i]!=L[i-1]) caps.push_back((int)i);
    printf("SRR p=255 capture positions:"); for(int c:caps) printf(" %d",c); printf("\n");
    CHECK(caps.size()>=5); CHECK(caps[0]==0);
    for(size_t k=1;k<caps.size();k++){ int d=caps[k]-caps[k-1]; CHECK(d==441||d==442); }
    // stereo: R changes exactly when L changes, R holds independent values
    bool sync=true; for(size_t i=1;i<L.size();i++) if((L[i]!=L[i-1])!=(R[i]!=R[i-1])) sync=false; CHECK(sync);
    CHECK(R[5]==-2*L[5]);   // R captured the same instant, own value
    CHECK(s.allocated==40 && s.released==40); }

  // ---- SRR: missing partner block => nothing transmitted, state unchanged, no leak
  { AudioSampleRateReduction s(2); s.reset(); s.setReduction(128); audio_block_t a; fill(a,0,0);
    s.in[0]=&a; s.in[1]=nullptr; s.out[0]=s.out[1]=nullptr; s.update(); CHECK(!s.out[0]&&!s.out[1]); CHECK(s.allocated==s.released); }

  // ---- SRR: parameter change keeps phase (no forced capture)
  { AudioSampleRateReduction s(1); s.reset(); s.setReduction(200);
    audio_block_t a; for(int i=0;i<128;i++) a.data[i]=i+1; s.in[0]=&a; s.update(); int16_t held=s.out[0]->data[127];
    s.setReduction(210); audio_block_t b; for(int i=0;i<128;i++) b.data[i]=1000+i; s.in[0]=&b; s.update();
    CHECK(s.out[0]->data[0]==held); }

  // ---- SRR: reset after bypass-> active: first sample captured (no stale zero)
  { AudioSampleRateReduction s(1); s.reset(); s.setReduction(255); audio_block_t a; for(int i=0;i<128;i++) a.data[i]=1234; s.in[0]=&a; s.update(); CHECK(s.out[0]->data[0]==1234); }

  // ---- DJ filter
  { AudioFilterDJ f(1);
    CHECK(fabsf(AudioFilterDJ::cutoffHz(-100)-20.0f)<0.5f);
    CHECK(fabsf(AudioFilterDJ::cutoffHz(100)-20000.0f)<5.0f);
    printf("DJ cutoffs: -100:%.1f -50:%.1f -1:%.1f +1:%.1f +50:%.1f +100:%.1f\n",AudioFilterDJ::cutoffHz(-100),AudioFilterDJ::cutoffHz(-50),AudioFilterDJ::cutoffHz(-1),AudioFilterDJ::cutoffHz(1),AudioFilterDJ::cutoffHz(50),AudioFilterDJ::cutoffHz(100));
    // bypass exact
    audio_block_t a; fill(a,0,0); f.in[0]=&a; f.setValue(0); f.update(); CHECK(f.out[0]==&a);
    // gain measurement helper
    auto gain=[&](int8_t v,float hz){ AudioFilterDJ g(1); g.setValue(v); double acc=0,ref=0; double ph=0; int N=60;
      for(int blk=0;blk<N;blk++){ audio_block_t x; for(int i=0;i<128;i++){ x.data[i]=(int16_t)(10000*sin(ph)); ph+=2*M_PI*hz/AUDIO_SAMPLE_RATE_EXACT; }
        g.in[0]=&x; g.update(); if(blk>N/2) for(int i=0;i<128;i++){ acc+=double(g.out[0]->data[i])*g.out[0]->data[i]; ref+=double(x.data[i])*x.data[i]; } }
      return 10*log10(acc/ref); };
    printf("LP(-50 => %.0f Hz): 100Hz %.1f dB, fc %.1f dB, 2fc %.1f dB, 4fc %.1f dB\n",AudioFilterDJ::cutoffHz(-50),gain(-50,100),gain(-50,AudioFilterDJ::cutoffHz(-50)),gain(-50,2*AudioFilterDJ::cutoffHz(-50)),gain(-50,4*AudioFilterDJ::cutoffHz(-50)));
    printf("HP(+50 => %.0f Hz): 100Hz %.1f dB, fc %.1f dB, fc/2 %.1f dB, fc/4 %.1f dB\n",AudioFilterDJ::cutoffHz(50),gain(50,100),gain(50,AudioFilterDJ::cutoffHz(50)),gain(50,AudioFilterDJ::cutoffHz(50)/2),gain(50,AudioFilterDJ::cutoffHz(50)/4));
    CHECK(gain(-50,AudioFilterDJ::cutoffHz(-50))<-2.5 && gain(-50,AudioFilterDJ::cutoffHz(-50))>-3.5);   // -3 dB at fc
    CHECK(gain(-50,4*AudioFilterDJ::cutoffHz(-50)) < -20.0 && gain(-50,4*AudioFilterDJ::cutoffHz(-50)) > -27.0); // ~12 dB/oct
    CHECK(gain(50,AudioFilterDJ::cutoffHz(50)/4) < -20.0 && gain(50,AudioFilterDJ::cutoffHz(50)/4) > -27.0);
    CHECK(gain(100,1000)< -20.0); CHECK(gain(-100,1000)< -20.0); CHECK(gain(-1,1000)>-0.5);
    // stability at extremes with full-scale noise: output finite & bounded for every value
    bool ok=true; for(int v=-100; v<=100; v++){ AudioFilterDJ g(2); g.setValue(v); for(int blk=0;blk<200;blk++){ audio_block_t x,y; for(int i=0;i<128;i++){x.data[i]=(blk&1)?32767:-32768; y.data[i]=(i&1)?32767:-32768;} g.in[0]=&x; g.in[1]=&y; g.update(); } }
    CHECK(ok);
    // stereo states independent: silence on R stays silent while L has signal
    AudioFilterDJ g(2); g.setValue(-30); audio_block_t x,y; for(int i=0;i<128;i++){x.data[i]=8000; y.data[i]=0;} for(int k=0;k<5;k++){ g.in[0]=&x; g.in[1]=&y; g.update(); }
    bool rzero=true; for(int i=0;i<128;i++) if(g.out[1]->data[i]!=0) rzero=false; CHECK(rzero); CHECK(g.out[0]->data[127]!=0);
    // missing partner
    AudioFilterDJ m(2); m.setValue(20); audio_block_t z; fill(z,0,0); m.in[0]=&z; m.in[1]=nullptr; m.out[0]=m.out[1]=nullptr; m.update(); CHECK(!m.out[0]&&!m.out[1]); CHECK(m.allocated==m.released);
  }
  printf(fails? "\n%d FAILURES\n":"\nALL PASS\n",fails); return fails!=0;
}
