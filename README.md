# BBB_dpram 작업환경 세팅
- BBB OS setting : beagleboard.org 에서 'AM3358 Debian 10.3 2020-04-06 4GB SD IoT' 을 다운로드 및 SD card 에 write
   - 커널 버전은 4.19.94

*** 이후 모든 작업은 BBB 상에서 진행

- Xenomai 3.1 설치 : https://mairacanal.github.io/install-xenomai-beaglebone-black/ 링크 참고
    - 'linux-image-4.19.94-ti-xenomai-r64' 를 설치
    - xenomai library 설정을 위해 아래의 과정을 수행

    ```
    $ cp /etc/ld.so.conf.d/libc.conf /etc/ld.so.conf.d/xenomai.conf
    $ vim /etc/ld.so.conf.d/xenomai.conf

    아래와 같이 설정된 내용을
    #libc default configuration
    /usr/local/lib
    
    다음과 같이 변경한다.
    # xenomai libs
    /usr/xenomai/lib
    ```
    
    - 헤더 include 오류 해결을 위해 '/usr/xenomai/include/trank/rtdm/rtdm.h' 에 있는 #include_next 내용을 아래와 같이 수정
    - #include_next "/usr/xenoma/include/rtdm/rtdm.h"

- 커널 빌드 환경을 갖추기 위해 header 파일 설치
    - apt-cache search linux-headers-$(uname -r)
    - apt-get install linux-headers-$(uname-r)
    - 위의 두 command 를 실행

- Device tree 빌드 환경을 갖추기 위해 아래의 과정을 수행
    - root 권한 상태에서 진행
    - wget -c https://raw.github.com/RobertCNelson/tools/master/pkgs/dtc.sh
    - chmod +x dtc.sh
    - ./dtc.sh

- github 에서 모든 폴더 (dpram_src, driver, dtc_build)를 BBB 로 다운로드(github) 또는 copy (scp 사용)
    - dtc_build 폴더로 이동하여 build.sh 수행
    - driver 폴더로 이동하여 make 수행
    - dpram_src 하위 각 폴더로 이동하여 make 수행

- BBB 의 uEnv.txt 수정 (/boot/uEnv.txt)
    - 'enable_uboot_cape_universal = 1' 을 주석처리
    - 'disable_uboot_overlay_emmc = 1' 을 활성화
    - 'disable_uboot_overlay_video = 1' 을 활성화
    - 'disable_uboot_overlay_audio = 1' 을 활성화
    - 'disable_uboot_overlay_wireless = 1' 을 활성화
    - 'disable_uboot_overlay_adc = 1' 을 활성화
    - 'enable_uboot_overlays = 1' 을 활성화
    - 'dtb_overlay=/lib/firmware/dualmemory-00A0.dtbo' line 을 추가

- 재부팅 수행

# BBB-DPRAM 연동 테스트
1. BBB power on 후 driver 폴더로 이동하여 'insmod dpram.ko' 수행
2. dpram_src 의 rtdm_test_write 폴더로 이동하여 './rtdm_test_write' 수행
3. dpram_src 의 rtdm_test_read 폴더로 이동하여 './rtdm_test_read' 수행
4. write/read 테스트 결과 확인