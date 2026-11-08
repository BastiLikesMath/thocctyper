#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <ApplicationServices/ApplicationServices.h>
#include <AudioToolbox/AudioToolbox.h>

// press sounds
static SystemSoundID sound_generic_1;
static SystemSoundID sound_generic_2;
static SystemSoundID sound_generic_3;
static SystemSoundID sound_spacebar;
static SystemSoundID sound_backspace;
static SystemSoundID sound_enter;

// release sounds
static SystemSoundID sound_generic_release;
static SystemSoundID sound_spacebar_release;
static SystemSoundID sound_backspace_release;
static SystemSoundID sound_enter_release;


SystemSoundID* get_random_generic_sound()
{
    int rnd = rand()%3;
    switch(rnd)
    {
        case 0:
            return &sound_generic_1;

        case 1:
            return &sound_generic_2;

        case 2:
            return &sound_generic_3;

        default:
            return &sound_generic_1;
    }
}

int init_audio(const char *path, SystemSoundID* sound)
{
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(
        NULL, (const UInt8 *)path, strlen(path), false
    );

    if (!url)
        return -1;

    OSStatus status = AudioServicesCreateSystemSoundID(
        url, sound
    );

    CFRelease(url);
    return status == noErr ? 0 : -1;
}

void play_audio(SystemSoundID* sound)
{
    AudioServicesPlaySystemSound(*sound);
}

void cleanup_audio(void)
{
    AudioServicesDisposeSystemSoundID(sound_generic_1);
    AudioServicesDisposeSystemSoundID(sound_generic_2);
    AudioServicesDisposeSystemSoundID(sound_generic_3);
    AudioServicesDisposeSystemSoundID(sound_spacebar);
    AudioServicesDisposeSystemSoundID(sound_enter);
    AudioServicesDisposeSystemSoundID(sound_generic_release);
    AudioServicesDisposeSystemSoundID(sound_spacebar_release);
    AudioServicesDisposeSystemSoundID(sound_backspace_release);
    AudioServicesDisposeSystemSoundID(sound_enter_release);
}

// Wird bei jedem Tastendruck aufgerufen
CGEventRef callback(CGEventTapProxy proxy,
                    CGEventType type,
                    CGEventRef event,
                    void *userInfo)
{


    if (type == kCGEventKeyDown) {
        CGKeyCode key = CGEventGetIntegerValueField(
            event, kCGKeyboardEventKeycode
        );

        printf("Keycode press: %d\n", (unsigned)key);
        if((unsigned)key == 36)
        {
            play_audio(&sound_enter);
        }
        else if ((unsigned)key == 49)
        {
            play_audio(&sound_spacebar);
        }
        else if ((unsigned)key == 51)
        {
            play_audio(&sound_backspace);
        }
        else 
        {
            play_audio(get_random_generic_sound());
        }
        fflush(stdout);
    }

    if (type == kCGEventKeyUp) {
        CGKeyCode key = CGEventGetIntegerValueField(
            event, kCGKeyboardEventKeycode
        );

        printf("Keycode release: %d\n", (unsigned)key);
        if((unsigned)key == 36)
        {
            play_audio(&sound_enter_release);
        }
        else if ((unsigned)key == 49)
        {
            play_audio(&sound_spacebar_release);
        }
        else if ((unsigned)key == 51)
        {
            play_audio(&sound_backspace_release);
        }
        else 
        {
            play_audio(&sound_generic_release);
        }
        fflush(stdout);
    }
    
    return event;
}

int init_audio_relative(const char *directory, const char *file, SystemSoundID *sound)
{
    char path[1024];
    int n = snprintf(path, sizeof(path), "%s/%s", directory, file);
    if(n < 0 || (size_t)n >= sizeof(path))
        return -1;
    return init_audio(path, sound);
    
}

int main(int argc, char* argv[])
{

    char *directory; 
    if(argc > 1)
    {
        directory = argv[1];
    }
    else
    {
        directory = "/Users/bastianswitajski/Git/thocctyper/audio/alpaca/";
    }

    if(init_audio_relative(directory, "press/GENERIC_R1.mp3", &sound_generic_1) != 0 
    || init_audio_relative(directory, "press/GENERIC_R2.mp3", &sound_generic_2) != 0 
    || init_audio_relative(directory, "press/GENERIC_R3.mp3", &sound_generic_3) != 0 
    || init_audio_relative(directory, "press/BACKSPACE.mp3", &sound_backspace) != 0 
    || init_audio_relative(directory, "press/SPACE.mp3", &sound_spacebar) != 0 
    || init_audio_relative(directory, "press/ENTER.mp3", &sound_enter) != 0 
    || init_audio_relative(directory, "release/GENERIC.mp3", &sound_generic_release) != 0 
    || init_audio_relative(directory, "release/SPACE.mp3", &sound_spacebar_release) != 0 
    || init_audio_relative(directory, "release/ENTER.mp3", &sound_enter_release) != 0 
    || init_audio_relative(directory, "release/BACKSPACE.mp3", &sound_backspace_release) != 0 
    )
    {
        fprintf(stderr, "Audio konnte nicht erstellt werden.\n");
        return 1;
    }

    CGEventMask mask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp);
    CFMachPortRef tap = CGEventTapCreate(
        kCGSessionEventTap,
        kCGHeadInsertEventTap,
        kCGEventTapOptionListenOnly,
        mask,
        callback,
        NULL
    );

    if (tap == NULL) {
        fprintf(stderr, "Event Tap konnte nicht erstellt werden.\n");
        return 1;
    }

    CFRunLoopSourceRef source =
        CFMachPortCreateRunLoopSource(NULL, tap, 0);

    if (source == NULL) {
        CFRelease(tap);
        return 1;
    }

    CFRunLoopAddSource(
        CFRunLoopGetCurrent(),
        source,
        kCFRunLoopCommonModes
    );

    // Wartet auf Ereignisse, ohne CPU-Busy-Waiting
    CFRunLoopRun();

    CFRelease(source);
    CFRelease(tap);

    cleanup_audio();
    return 0;
}
