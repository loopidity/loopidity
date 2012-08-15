
#ifndef _LOOPIDITY_H_
#define _LOOPIDITY_H_


// DEBUG
#define DEBUG 1
#define MEMORY_CHECK 1 // if 0 choose DEFAULT_BUFFER_SIZE wisely
#define INIT_LOOPIDITY 1
#define INIT_JACK 1
#define LOOP_COUNTER 1
#define DSP 1
#define PASSTHRU 0
// TODO: user defined buffer sizes
#define STATIC_BUFFER_SIZE 1
// DEBUG end

// GUI DEBUG
#define DRAW_STATUS 1
#define DRAW_SCENES 1
#define DRAW_SCOPES 1

#if DRAW_STATUS
#define DRAW_MODE 0
/*
#define UPDATE_MEMORY 0
#define UPDATE_LOOP_PROGRESS 0
#define UPDATE_VU 0
#define VARDUMP DEBUG && 0
*/
#endif

#if DRAW_SCENES
#define DRAW_SCENE_SCOPE 1
#define DRAW_HISTOGRAMS 1
#define DRAW_MUTED_HISTOGRAMS 1
#define DRAW_PEAK_RINGS 1
#define DRAW_LOOPS 1
#define DRAW_MUTED_LOOPS 0
#define DRAW_RECORDING_LOOP DRAW_LOOPS && 1
#define DRAW_CURRENT_SCENE_INDICATOR 1
#define DRAW_DEBUG_TEXT 1
#endif

#define SCAN_LOOP_PEAKS_DATA 1
#define SCAN_TRANSIENT_PEAKS_DATA 1
// GUI DEBUG end


// dependencies
#ifdef __cplusplus
#include <cstdlib>
#include <vector>
#include <string>
#else
#include <stdlib.h>
#include <vector.h>
#include <string.h>
#endif

#ifdef __APPLE__
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_ttf.h>
#else
#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_rotozoom.h>
#include <SDL_ttf.h>
#endif

#include <X11/Xlib.h>


// quantities
#define DEFAULT_BUFFER_SIZE 33554432 // 2^25 (approx 3 min @ 48k)
//#define DEFAULT_BUFFER_SIZE 25165824 // 1024 * 1024 * 24 (approx 135 sec @ 48k)
//#define DEFAULT_BUFFER_SIZE 16777216 // 2^24 (approx 90 sec @ 48k)
//#define DEFAULT_BUFFER_SIZE 8388608 // 2^23 (approx 45 sec @ 48k)
//#define DEFAULT_BUFFER_SIZE 2097152 // 2^21 (approx 10 sec @ 48k)
//#define DEFAULT_BUFFER_SIZE 1048576 // 2^20 (approx 5 sec @ 48k)

// TODO: implement setting N_CHANNELS via cmd line arg - GetTransientPeaks and updateVUMeters are especially brittle now
#define N_INPUT_CHANNELS 2 // TODO: nyi - only used for memory check and scope cache
#define N_OUTPUT_CHANNELS 2 // TODO: nyi - only used for N_PORTS
#define N_PORTS N_INPUT_CHANNELS + N_OUTPUT_CHANNELS // TODO: nyi - only used for scope cache
#define N_SCENES 3
#define N_LOOPS 9 // N_LOOPS_PER_SCENE

// string constants
#define APP_NAME "Loopidity"
//#define CONNECT_ARG "--connect"
#define MONITOR_ARG "--nomon"
#define SCENE_CHANGE_ARG "--noautoscenechange"
#define JACK_FAIL_MSG "ERROR: Could not register JACK client - quitting"
#define FREEMEM_FAIL_MSG "ERROR: Could not determine available memory - quitting"
#define ZERO_BUFFER_SIZE_MSG "ERROR: initBufferSize is zero - quitting"
#define INSUFFICIENT_MEMORY_MSG "ERROR: Insufficient memory - quitting"
#define OUT_OF_MEMORY_MSG "ERROR: Out of Memory"

// error states
#define JACK_INIT_SUCCESS 0
#define JACK_FAIL 1
#define JACK_MEM_FAIL 2

// aliases
#define SAMPLE jack_default_audio_sample_t


#include "jack_io.h"
#include "loopidity_sdl.h"
#include "scene_sdl.h"


using namespace std ;


class Loop
{
friend class SceneSdl ; // TODO: Loop class is private - pass in peaks[currentPeakN] to drawScene()
	friend class Loopidity ;
	friend class Scene ;
	friend class JackIO ;

	public:

		SAMPLE getPeakFine(unsigned int peakN) ;
		SAMPLE getPeakCourse(unsigned int peakN) ;

	private:

		Loop(unsigned int nFrames) ;
		~Loop() ;

		SAMPLE* buffer1 ;
		SAMPLE* buffer2 ;
		SAMPLE peaksFine[N_PEAKS_FINE] ;
		SAMPLE peaksCourse[N_PEAKS_COURSE] ;
} ;


class Scene
{
	friend class Loopidity ;
	friend class LoopiditySdl ;
	friend class SceneSdl ;
	friend class JackIO ;

	public:

// DEBUG
void makeMainDbgText(char* dbg) ;
// DEBUG end

		// scene progress
		unsigned int getCurrentPeakN() ;
		float getCurrentSeconds() ;
		float getTotalSeconds() ;

	private:

		typedef Scene CLASSNAME ;
		Scene(unsigned int scenen , unsigned int recordBufferSize) ;
		virtual ~Scene() {}

		// identity
		unsigned int sceneN ;

		// GUI
		SceneSdl* sceneGui ;

		// audio data
		vector<Loop*> loops ;

		// peaks cache
		float hiScenePeaks[N_PEAKS_FINE] ; // the loudest of the currently playing samples in the current scene
		float hiLoopPeaks[N_LOOPS] ; // the loudest sample for each loop of the current scene
		float highestScenePeak ; // the loudest of all samples in all loops of the current scene (nyi)

		// sample metedata
		static unsigned int RecordBufferSize ;
		static unsigned int SampleRate ;
		static unsigned int FrameSize ;
		static unsigned int NFramesPerPeriod ;

		// buffer iteration
		unsigned int nFrames ;
		unsigned int nFramesPerPeak ;
		unsigned int frameN ;
		unsigned int nBytes ;

		// recording state
		static bool IsRecording ;
		bool isSaveLoop ;
		bool isPulseExist ;

		// audio data
		void addLoop(Loop* newLoop) ;
		void deleteLoop() ;
		void reset() ;

		// peaks cache
		void scanPeaks(Loop* loop , unsigned int loopN) ;
		void rescanPeaks() ;

		// recording state
		void setStateIndicators() ;
    void setMode() ;
    void sceneChanged() ;

		// getters/setters
		static void SetMetaData(unsigned int sampleRate , unsigned int frameSize , unsigned int nFramesPerPeriod) ;
		void setSceneGui(SceneSdl* aSceneGui) ;
		bool getIsRecording() ;
    unsigned int getLoopPos() ;
} ;


class Loopidity
{
	public:

		// user actions
		static bool SetRecordBufferSize(unsigned int recordBufferSize) ;
    static bool Init(bool isMonitorInputs) ;
    static void ToggleAutoSceneChange() ;
		static void ToggleScene() ;
    static void SetMode() ;
    static void DeleteLoop() ;
    static void ResetCurrentScene() ;
		static void Reset() ;

		// getters/setters
		static void SetMetaData(unsigned int sampleRate , unsigned int frameSize , unsigned int nFramesPerPeriod) ;
		static void SetSceneGui(SceneSdl* sceneGui , unsigned int sceneN) ;
		static Scene* GetCurrentScene() ;
		static unsigned int GetCurrentSceneN() ;
		static unsigned int GetNextSceneN() ;
    static unsigned int GetLoopPos() ;
		static bool GetIsSaveLoop() ;
		static bool GetIsPulseExist() ;
		static void SetNFramesPerPeriod(unsigned int nFrames) ;
		static vector<SAMPLE>* GetPeaksInCache() ;
		static vector<SAMPLE>* GetPeaksOutCache() ;
		static SAMPLE* GetTransientPeaksCache() ;
		static SAMPLE* GetTransientPeakIn() ;
		static SAMPLE* GetTransientPeakOut() ;
		static SAMPLE GetPeak(SAMPLE* buffer , unsigned int nFrames) ;

		// helpers
		static void SceneChanged(Scene* nextScene) ;
		static void ScanTransientPeaks() ;
		static void OOM() ;

	private:

		// recording state
		static Scene* CurrentScene ;
		static unsigned int NextSceneN ;
		static bool IsAutoSceneChange ;

		// audio data
		static Scene* Scenes[N_SCENES] ;
		static unsigned int NFramesPerPeriod ;
		static unsigned int NFramesPerGuiInterval ;

		// transient sample data
		static SAMPLE* RecordBuffer1 ;
		static SAMPLE* RecordBuffer2 ;
		static vector<SAMPLE> PeaksIn ;
		static vector<SAMPLE> PeaksOut ;
		static SAMPLE TransientPeaks[N_PORTS] ;
		static SAMPLE TransientPeakInMix ;
		static SAMPLE TransientPeakOutMix ;
} ;


#endif // #ifndef _LOOPIDITY_H_
