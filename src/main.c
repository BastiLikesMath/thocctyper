#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <ApplicationServices/ApplicationServices.h>
#include <AudioToolbox/AudioToolbox.h>

static SystemSoundID sound_generic_1;
static SystemSoundID sound_generic_2;
static SystemSoundID sound_generic_3;
static SystemSoundID sound_spacebar;
static SystemSoundID sound_enter;

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

        printf("Keycode: %u\n", (unsigned)key);

        if((unsigned)key == 36)
        {
            play_audio(&sound_enter);
        }
        else if ((unsigned)key == 49)
        {
            play_audio(&sound_spacebar);
        }
        else 
        {
            play_audio(&sound_generic_1);
        }
        fflush(stdout);
    }
    
    return event;
}

int main(void)
{
    if(init_audio("/Users/bastianswitajski/Git/thocctyper/audio/alpaca/press/GENERIC_R1.mp3", &sound_generic_1) != 0)
    {
        fprintf(stderr, "Audio konnte nicht erstellt werden.\n");
        return 1;
    }

    if(init_audio("/Users/bastianswitajski/Git/thocctyper/audio/alpaca/press/BACKSPACE.mp3", &sound_spacebar) != 0)
    {
        fprintf(stderr, "Audio konnte nicht erstellt werden.\n");
        return 1;
    }

    if(init_audio("/Users/bastianswitajski/Git/thocctyper/audio/alpaca/press/ENTER.mp3", &sound_enter) != 0)
    {
        fprintf(stderr, "Audio konnte nicht erstellt werden.\n");
        return 1;
    }

    CGEventMask mask = CGEventMaskBit(kCGEventKeyDown);
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
    return 0;
}
