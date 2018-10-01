#define	_GNU_SOURCE

#include <stdio.h>
#include <wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

//#define fgfs
//#define sd2_launch
//#define pillar_launch
//#define	longdark_launch
//#define eclipse
//#define vegastrike_launch
//#define firefox
//#define	swriter
//#define	impress
//#define	SOMA_launch
//#define	EoCApp_launch
//#define pillar_loading_1
//#define pillar_loading_2
#define vegastrike_loading_1
//#define sd2_loading_1
//#define	SOMA_loading_1
//#define	SOMA_loading_2
//#define	witcher2_launch
//#define	witcher2_loading_1
//#define	witcher2_loading_2
//#define	longdark_loading_1
//#define	WL2_launch
//#define	WL2_loading_1
//#define	WL2_loading_2
//#define	EoCApp_loading_1
//#define	EoCApp_loading_2

#ifdef fgfs
#define PATH	"/usr/lib/x86_64-linux-gnu/libflite_cmu_us_kal.so.2.1"
#define OFFSET	271040
#define LENGTH	531072
#endif

#ifdef sd2_launch
#define PATH	"/usr/share/games/speed-dreams-2/data/music/main.ogg"
#define	OFFSET	0
#define	LENGTH	9453488
#endif

#ifdef pillar_launch
#define PATH	"/home/melody/GOG Games/Pillars of Eternity/game/PillarsOfEternity_Data/sharedassets12.assets.resS"
#define	OFFSET	22614015
#define	LENGTH	131072
#endif

#ifdef	longdark_launch
#define	PATH	"/home/melody/The_Long_Dark/TheLongDark/tld_Data/StreamingAssets/Audio/GeneratedSoundBanks/Linux/428339376.wem"
#define	OFFSET	0
#define	LENGTH	1507328
#endif

#ifdef eclipse
#define	PATH	"/usr/lib/x86_64-linux-gnu/libwebkit2gtk-4.0.so.37.24.9"
#define	OFFSET	31608832
#define LENGTH	131072
#endif

#ifdef vegastrike_launch
#define	PATH	"/usr/share/games/vegastrike/modules/generate_dyn_universe.pyc"
#define OFFSET	0
#define	LENGTH	0
#endif

#ifdef	firefox
#define	PATH	"/home/melody/.mozilla/firefox/tscq5ttn.default/shield-preference-experiments.json"
#define	OFFSET	0
#define	LENGTH	4096
#endif

#ifdef	swriter
#define	PATH	"/usr/lib/libreoffice/program/libswlo.so"
#define	OFFSET	6590464
#define	LENGTH	73728
#endif

#ifdef	impress
#define	PATH	"/usr/lib/libreoffice/program/liblnthlo.so"
#define	OFFSET	16384
#define	LENGTH	57344
#endif

#ifdef	SOMA_launch
#define	PATH	"/home/melody/GOG Games/SOMA/game/music/Themes/Menu_Music.ogg"
#define	OFFSET	407904
#define	LENGTH	331072
#endif

#ifdef	EoCApp_launch
#define	PATH	"/home/melody/GOG Games/Divinity Original Sin Enhanced Edition/game/Data/Sound.pak"
#define	OFFSET	367980544
#define	LENGTH	131072
#endif

#ifdef	EoCApp_loading_1
#define	PATH	"/home/melody/GOG Games/Divinity Original Sin Enhanced Edition/game/Data/Sound.pak"
#define	OFFSET	189681664
#define	LENGTH	131072
#endif

#ifdef	EoCApp_loading_2
#define	PATH	"/home/melody/GOG Games/Divinity Original Sin Enhanced Edition/game/Data/Sound_1.pak"
#define OFFSET	316297216
#define	LENGTH	131072
#endif

#ifdef	WL2_launch
#define	PATH	"/home/melody/Games/Wasteland2/game/WL2_Data/Managed/System.dll"
#define	OFFSET	20480
#define	LENGTH	262144
#endif

#ifdef	WL2_loading_1
#define	PATH	"/home/melody/Games/Wasteland2/game/WL2"
#define	OFFSET	12754944
#define	LENGTH	147456
#endif

#ifdef	WL2_loading_2
#define	PATH	"/home/melody/Games/Wasteland2/game/WL2_Data/resources.assets"
#define	OFFSET	117047296
#define	LENGTH	241664
#endif

#ifdef	longdark_loading_1
#define	PATH	"/home/melody/The_Long_Dark/TheLongDark/tld_Data/level77"
#define	OFFSET	0
#define	LENGTH	4321280
#endif

#ifdef	witcher2_launch
#define	PATH	"/home/melody/GOG Games/The Witcher 2 Assassins Of Kings Enhanced Edition/game/CookedPC/movies/prolog_loop.usm"
#define	OFFSET	0
#define	LENGTH	8388608
#endif

#ifdef	witcher2_loading_1
#define	PATH	"/home/melody/GOG Games/The Witcher 2 Assassins Of Kings Enhanced Edition/game/CookedPC/pack0.dzip"
#define	OFFSET	1157627904
#define	LENGTH	134217728
#endif

#ifdef	witcher2_loading_2
#define	PATH	"/home/melody/GOG Games/The Witcher 2 Assassins Of Kings Enhanced Edition/game/CookedPC/pack0.dzip"
#define	OFFSET	486539264
#define	LENGTH	134217728
#endif

#ifdef vegastrike_loading_1
#define PATH	"/usr/share/games/vegastrike/sprites/bases/ocean/ocean_exterior4.image"
#define OFFSET	0
#define LENGTH	0
#endif

#ifdef sd2_loading_1
#define	PATH	"/usr/lib/x86_64-linux-gnu/libnvidia-glcore.so.384.111"
#define	OFFSET	18862080
#define	LENGTH	524288
#endif

#ifdef pillar_loading_1
#define	PATH	"/home/melody/GOG Games/Pillars of Eternity/game/PillarsOfEternity_Data/assetbundles/prefabs/objectbundle/gammacamera.unity3d"
#define	OFFSET	0
#define	LENGTH	0
#endif

#ifdef pillar_loading_2
#define	PATH	"/home/melody/GOG Games/Pillars of Eternity/game/PillarsOfEternity_Data/sharedassets117.assets.resS"
#define	OFFSET	0
#define	LENGTH	0
#endif

#ifdef	SOMA_loading_1
#define	PATH	"/home/melody/GOG Games/SOMA/game/textures/screeneffects/water_drip_screen_nrm.dds"
#define	OFFSET	1327104
#define	LENGTH	65536
#endif

#ifdef	SOMA_loading_2
#define	PATH	"/home/melody/GOG Games/SOMA/game/sounds/level/00_06_lab_streams.fsb"
#define	OFFSET	20627456
#define	LENGTH	131072
#endif

int main(int argc, char **argv)
{
	int pid;
	int wait_status;
	int fd;
	off_t starting_offset = OFFSET;
	off_t length = LENGTH;

	if (argc < 2) {
		printf("Usage: ./coldstart <Target app's path>\n");
		exit(EXIT_FAILURE);
	}

	fd = open(PATH, O_RDONLY);

	while (1) {
		if (length > 131072) {
			if (posix_fadvise(fd, starting_offset, 131072, POSIX_FADV_DONTNEED) < 0) {
				perror("posix_fadvise");
			}
			starting_offset = starting_offset + 131072;
			length = length - 131072;
		}
		else {
			if (posix_fadvise(fd, starting_offset, length, POSIX_FADV_DONTNEED) < 0) {
				perror("posix_fadvise");
			}
			break;
		}
	}
	close(fd);

#ifdef	witcher2_loading_2
#define	PATH	"/home/melody/GOG Games/The Witcher 2 Assassins Of Kings Enhanced Edition/game/CookedPC/sounds/music/l01_music.fsb"
#define	OFFSET	0
#define	LENGTH	29736960
	starting_offset = OFFSET;
	length = LENGTH;

	fd = open(PATH, O_RDONLY);

	while (1) {
		if (length > 131072) {
			if (posix_fadvise(fd, starting_offset, 131072, POSIX_FADV_DONTNEED) < 0) {
				perror("posix_fadvise");
			}
			starting_offset = starting_offset + 131072;
			length = length - 131072;
		}
		else {
			if (posix_fadvise(fd, starting_offset, length, POSIX_FADV_DONTNEED) < 0) {
				perror("posix_fadvise");
			}
			break;
		}
	}
	close(fd);
#endif


	system("cat /proc/uptime");

	if ((pid = fork()) < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) {
		if (execv(argv[1], &argv[1]) < 0) {
			perror("execl");
			exit(EXIT_FAILURE);
		}
	}
	pid = waitpid(-1, &wait_status, __WALL);
	return 0;
	
}
