//
// Created by wangchenfei01 on 2021/8/21.
//
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <string>
#include"WeightedHashMap.h"
using namespace std;

#define crush_hashmix(a, b, c) do {			\
        a = a-b;  a = a-c;  a = a^(c>>13);	\
        b = b-c;  b = b-a;  b = b^(a<<8);	\
        c = c-a;  c = c-b;  c = c^(b>>13);	\
        a = a-b;  a = a-c;  a = a^(c>>12);	\
        b = b-c;  b = b-a;  b = b^(a<<16);	\
        c = c-a;  c = c-b;  c = c^(b>>5);	\
        a = a-b;  a = a-c;  a = a^(c>>3);	\
        b = b-c;  b = b-a;  b = b^(a<<10);	\
        c = c-a;  c = c-b;  c = c^(b>>15);	\
} while (0)


/*
 * RH_LH_tbl[2*k] = 2^48/(1.0+k/128.0)
 * RH_LH_tbl[2*k+1] = 2^48*log2(1.0+k/128.0)
 */
static int64_t __RH_LH_tbl[128*2+2] = {
        0x0001000000000000ll, 0x0000000000000000ll, 0x0000fe03f80fe040ll, 0x000002dfca16dde1ll,
        0x0000fc0fc0fc0fc1ll, 0x000005b9e5a170b4ll, 0x0000fa232cf25214ll, 0x0000088e68ea899all,
        0x0000f83e0f83e0f9ll, 0x00000b5d69bac77ell, 0x0000f6603d980f67ll, 0x00000e26fd5c8555ll,
        0x0000f4898d5f85bcll, 0x000010eb389fa29fll, 0x0000f2b9d6480f2cll, 0x000013aa2fdd27f1ll,
        0x0000f0f0f0f0f0f1ll, 0x00001663f6fac913ll, 0x0000ef2eb71fc435ll, 0x00001918a16e4633ll,
        0x0000ed7303b5cc0fll, 0x00001bc84240adabll, 0x0000ebbdb2a5c162ll, 0x00001e72ec117fa5ll,
        0x0000ea0ea0ea0ea1ll, 0x00002118b119b4f3ll, 0x0000e865ac7b7604ll, 0x000023b9a32eaa56ll,
        0x0000e6c2b4481cd9ll, 0x00002655d3c4f15cll, 0x0000e525982af70dll, 0x000028ed53f307eell,
        0x0000e38e38e38e39ll, 0x00002b803473f7adll, 0x0000e1fc780e1fc8ll, 0x00002e0e85a9de04ll,
        0x0000e070381c0e08ll, 0x0000309857a05e07ll, 0x0000dee95c4ca038ll, 0x0000331dba0efce1ll,
        0x0000dd67c8a60dd7ll, 0x0000359ebc5b69d9ll, 0x0000dbeb61eed19dll, 0x0000381b6d9bb29bll,
        0x0000da740da740dbll, 0x00003a93dc9864b2ll, 0x0000d901b2036407ll, 0x00003d0817ce9cd4ll,
        0x0000d79435e50d7all, 0x00003f782d7204d0ll, 0x0000d62b80d62b81ll, 0x000041e42b6ec0c0ll,
        0x0000d4c77b03531ell, 0x0000444c1f6b4c2dll, 0x0000d3680d3680d4ll, 0x000046b016ca47c1ll,
        0x0000d20d20d20d21ll, 0x000049101eac381cll, 0x0000d0b69fcbd259ll, 0x00004b6c43f1366all,
        0x0000cf6474a8819fll, 0x00004dc4933a9337ll, 0x0000ce168a772509ll, 0x0000501918ec6c11ll,
        0x0000cccccccccccdll, 0x00005269e12f346ell, 0x0000cb8727c065c4ll, 0x000054b6f7f1325all,
        0x0000ca4587e6b750ll, 0x0000570068e7ef5all, 0x0000c907da4e8712ll, 0x000059463f919deell,
        0x0000c7ce0c7ce0c8ll, 0x00005b8887367433ll, 0x0000c6980c6980c7ll, 0x00005dc74ae9fbecll,
        0x0000c565c87b5f9ell, 0x00006002958c5871ll, 0x0000c4372f855d83ll, 0x0000623a71cb82c8ll,
        0x0000c30c30c30c31ll, 0x0000646eea247c5cll, 0x0000c1e4bbd595f7ll, 0x000066a008e4788cll,
        0x0000c0c0c0c0c0c1ll, 0x000068cdd829fd81ll, 0x0000bfa02fe80bfbll, 0x00006af861e5fc7dll,
        0x0000be82fa0be830ll, 0x00006d1fafdce20all, 0x0000bd6910470767ll, 0x00006f43cba79e40ll,
        0x0000bc52640bc527ll, 0x00007164beb4a56dll, 0x0000bb3ee721a54ell, 0x000073829248e961ll,
        0x0000ba2e8ba2e8bbll, 0x0000759d4f80cba8ll, 0x0000b92143fa36f6ll, 0x000077b4ff5108d9ll,
        0x0000b81702e05c0cll, 0x000079c9aa879d53ll, 0x0000b70fbb5a19bfll, 0x00007bdb59cca388ll,
        0x0000b60b60b60b61ll, 0x00007dea15a32c1bll, 0x0000b509e68a9b95ll, 0x00007ff5e66a0ffell,
        0x0000b40b40b40b41ll, 0x000081fed45cbccbll, 0x0000b30f63528918ll, 0x00008404e793fb81ll,
        0x0000b21642c8590cll, 0x000086082806b1d5ll, 0x0000b11fd3b80b12ll, 0x000088089d8a9e47ll,
        0x0000b02c0b02c0b1ll, 0x00008a064fd50f2all, 0x0000af3addc680b0ll, 0x00008c01467b94bbll,
        0x0000ae4c415c9883ll, 0x00008df988f4ae80ll, 0x0000ad602b580ad7ll, 0x00008fef1e987409ll,
        0x0000ac7691840ac8ll, 0x000091e20ea1393ell, 0x0000ab8f69e2835all, 0x000093d2602c2e5fll,
        0x0000aaaaaaaaaaabll, 0x000095c01a39fbd6ll, 0x0000a9c84a47a080ll, 0x000097ab43af59f9ll,
        0x0000a8e83f5717c1ll, 0x00009993e355a4e5ll, 0x0000a80a80a80a81ll, 0x00009b79ffdb6c8bll,
        0x0000a72f0539782all, 0x00009d5d9fd5010bll, 0x0000a655c4392d7cll, 0x00009f3ec9bcfb80ll,
        0x0000a57eb50295fbll, 0x0000a11d83f4c355ll, 0x0000a4a9cf1d9684ll, 0x0000a2f9d4c51039ll,
        0x0000a3d70a3d70a4ll, 0x0000a4d3c25e68dcll, 0x0000a3065e3fae7dll, 0x0000a6ab52d99e76ll,
        0x0000a237c32b16d0ll, 0x0000a8808c384547ll, 0x0000a16b312ea8fdll, 0x0000aa5374652a1cll,
        0x0000a0a0a0a0a0a1ll, 0x0000ac241134c4e9ll, 0x00009fd809fd80a0ll, 0x0000adf26865a8a1ll,
        0x00009f1165e72549ll, 0x0000afbe7fa0f04dll, 0x00009e4cad23dd60ll, 0x0000b1885c7aa982ll,
        0x00009d89d89d89d9ll, 0x0000b35004723c46ll, 0x00009cc8e160c3fcll, 0x0000b5157cf2d078ll,
        0x00009c09c09c09c1ll, 0x0000b6d8cb53b0call, 0x00009b4c6f9ef03bll, 0x0000b899f4d8ab63ll,
        0x00009a90e7d95bc7ll, 0x0000ba58feb2703all, 0x000099d722dabde6ll, 0x0000bc15edfeed32ll,
        0x0000991f1a515886ll, 0x0000bdd0c7c9a817ll, 0x00009868c809868dll, 0x0000bf89910c1678ll,
        0x000097b425ed097cll, 0x0000c1404eadf383ll, 0x000097012e025c05ll, 0x0000c2f5058593d9ll,
        0x0000964fda6c0965ll, 0x0000c4a7ba58377cll, 0x000095a02568095bll, 0x0000c65871da59ddll,
        0x000094f2094f2095ll, 0x0000c80730b00016ll, 0x0000944580944581ll, 0x0000c9b3fb6d0559ll,
        0x0000939a85c4093all, 0x0000cb5ed69565afll, 0x000092f113840498ll, 0x0000cd07c69d8702ll,
        0x0000924924924925ll, 0x0000ceaecfea8085ll, 0x000091a2b3c4d5e7ll, 0x0000d053f6d26089ll,
        0x000090fdbc090fdcll, 0x0000d1f73f9c70c0ll, 0x0000905a38633e07ll, 0x0000d398ae817906ll,
        0x00008fb823ee08fcll, 0x0000d53847ac00a6ll, 0x00008f1779d9fdc4ll, 0x0000d6d60f388e41ll,
        0x00008e78356d1409ll, 0x0000d8720935e643ll, 0x00008dda5202376all, 0x0000da0c39a54804ll,
        0x00008d3dcb08d3ddll, 0x0000dba4a47aa996ll, 0x00008ca29c046515ll, 0x0000dd3b4d9cf24bll,
        0x00008c08c08c08c1ll, 0x0000ded038e633f3ll, 0x00008b70344a139cll, 0x0000e0636a23e2eell,
        0x00008ad8f2fba939ll, 0x0000e1f4e5170d02ll, 0x00008a42f870566all, 0x0000e384ad748f0ell,
        0x000089ae4089ae41ll, 0x0000e512c6e54998ll, 0x0000891ac73ae982ll, 0x0000e69f35065448ll,
        0x0000888888888889ll, 0x0000e829fb693044ll, 0x000087f78087f781ll, 0x0000e9b31d93f98ell,
        0x00008767ab5f34e5ll, 0x0000eb3a9f019750ll, 0x000086d905447a35ll, 0x0000ecc08321eb30ll,
        0x0000864b8a7de6d2ll, 0x0000ee44cd59ffabll, 0x000085bf37612cefll, 0x0000efc781043579ll,
        0x0000853408534086ll, 0x0000f148a170700all, 0x000084a9f9c8084bll, 0x0000f2c831e44116ll,
        0x0000842108421085ll, 0x0000f446359b1353ll, 0x0000839930523fbfll, 0x0000f5c2afc65447ll,
        0x000083126e978d50ll, 0x0000f73da38d9d4all, 0x0000828cbfbeb9a1ll, 0x0000f8b7140edbb1ll,
        0x0000820820820821ll, 0x0000fa2f045e7832ll, 0x000081848da8faf1ll, 0x0000fba577877d7dll,
        0x0000810204081021ll, 0x0000fd1a708bbe11ll, 0x0000808080808081ll, 0x0000fe8df263f957ll,
        0x0000800000000000ll, 0x0000ffff00000000ll,
        };


/*
 * LL_tbl[k] = 2^48*log2(1.0+k/2^15)
 */
static int64_t __LL_tbl[256] = {
        0x0000000000000000ull, 0x00000002e2a60a00ull, 0x000000070cb64ec5ull, 0x00000009ef50ce67ull,
        0x0000000cd1e588fdull, 0x0000000fb4747e9cull, 0x0000001296fdaf5eull, 0x0000001579811b58ull,
        0x000000185bfec2a1ull, 0x0000001b3e76a552ull, 0x0000001e20e8c380ull, 0x0000002103551d43ull,
        0x00000023e5bbb2b2ull, 0x00000026c81c83e4ull, 0x00000029aa7790f0ull, 0x0000002c8cccd9edull,
        0x0000002f6f1c5ef2ull, 0x0000003251662017ull, 0x0000003533aa1d71ull, 0x0000003815e8571aull,
        0x0000003af820cd26ull, 0x0000003dda537faeull, 0x00000040bc806ec8ull, 0x000000439ea79a8cull,
        0x0000004680c90310ull, 0x0000004962e4a86cull, 0x0000004c44fa8ab6ull, 0x0000004f270aaa06ull,
        0x0000005209150672ull, 0x00000054eb19a013ull, 0x00000057cd1876fdull, 0x0000005aaf118b4aull,
        0x0000005d9104dd0full, 0x0000006072f26c64ull, 0x0000006354da3960ull, 0x0000006636bc441aull,
        0x0000006918988ca8ull, 0x0000006bfa6f1322ull, 0x0000006edc3fd79full, 0x00000071be0ada35ull,
        0x000000749fd01afdull, 0x00000077818f9a0cull, 0x0000007a6349577aull, 0x0000007d44fd535eull,
        0x0000008026ab8dceull, 0x00000083085406e3ull, 0x00000085e9f6beb2ull, 0x00000088cb93b552ull,
        0x0000008bad2aeadcull, 0x0000008e8ebc5f65ull, 0x0000009170481305ull, 0x0000009451ce05d3ull,
        0x00000097334e37e5ull, 0x0000009a14c8a953ull, 0x0000009cf63d5a33ull, 0x0000009fd7ac4a9dull,
        0x000000a2b07f3458ull, 0x000000a59a78ea6aull, 0x000000a87bd699fbull, 0x000000ab5d2e8970ull,
        0x000000ae3e80b8e3ull, 0x000000b11fcd2869ull, 0x000000b40113d818ull, 0x000000b6e254c80aull,
        0x000000b9c38ff853ull, 0x000000bca4c5690cull, 0x000000bf85f51a4aull, 0x000000c2671f0c26ull,
        0x000000c548433eb6ull, 0x000000c82961b211ull, 0x000000cb0a7a664dull, 0x000000cdeb8d5b82ull,
        0x000000d0cc9a91c8ull, 0x000000d3ada20933ull, 0x000000d68ea3c1ddull, 0x000000d96f9fbbdbull,
        0x000000dc5095f744ull, 0x000000df31867430ull, 0x000000e2127132b5ull, 0x000000e4f35632eaull,
        0x000000e7d43574e6ull, 0x000000eab50ef8c1ull, 0x000000ed95e2be90ull, 0x000000f076b0c66cull,
        0x000000f35779106aull, 0x000000f6383b9ca2ull, 0x000000f918f86b2aull, 0x000000fbf9af7c1aull,
        0x000000feda60cf88ull, 0x00000101bb0c658cull, 0x000001049bb23e3cull, 0x000001077c5259afull,
        0x0000010a5cecb7fcull, 0x0000010d3d81593aull, 0x000001101e103d7full, 0x00000112fe9964e4ull,
        0x00000115df1ccf7eull, 0x00000118bf9a7d64ull, 0x0000011ba0126eadull, 0x0000011e8084a371ull,
        0x0000012160f11bc6ull, 0x000001244157d7c3ull, 0x0000012721b8d77full, 0x0000012a02141b10ull,
        0x0000012ce269a28eull, 0x0000012fc2b96e0full, 0x00000132a3037daaull, 0x000001358347d177ull,
        0x000001386386698cull, 0x0000013b43bf45ffull, 0x0000013e23f266e9ull, 0x00000141041fcc5eull,
        0x00000143e4477678ull, 0x00000146c469654bull, 0x00000149a48598f0ull, 0x0000014c849c117cull,
        0x0000014f64accf08ull, 0x0000015244b7d1a9ull, 0x0000015524bd1976ull, 0x0000015804bca687ull,
        0x0000015ae4b678f2ull, 0x0000015dc4aa90ceull, 0x00000160a498ee31ull, 0x0000016384819134ull,
        0x00000166646479ecull, 0x000001694441a870ull, 0x0000016c24191cd7ull, 0x0000016df6ca19bdull,
        0x00000171e3b6d7aaull, 0x00000174c37d1e44ull, 0x00000177a33dab1cull, 0x0000017a82f87e49ull,
        0x0000017d62ad97e2ull, 0x00000180425cf7feull, 0x00000182b07f3458ull, 0x0000018601aa8c19ull,
        0x00000188e148c046ull, 0x0000018bc0e13b52ull, 0x0000018ea073fd52ull, 0x000001918001065dull,
        0x000001945f88568bull, 0x000001973f09edf2ull, 0x0000019a1e85ccaaull, 0x0000019cfdfbf2c8ull,
        0x0000019fdd6c6063ull, 0x000001a2bcd71593ull, 0x000001a59c3c126eull, 0x000001a87b9b570bull,
        0x000001ab5af4e380ull, 0x000001ae3a48b7e5ull, 0x000001b11996d450ull, 0x000001b3f8df38d9ull,
        0x000001b6d821e595ull, 0x000001b9b75eda9bull, 0x000001bc96961803ull, 0x000001bf75c79de3ull,
        0x000001c254f36c51ull, 0x000001c534198365ull, 0x000001c81339e336ull, 0x000001caf2548bd9ull,
        0x000001cdd1697d67ull, 0x000001d0b078b7f5ull, 0x000001d38f823b9aull, 0x000001d66e86086dull,
        0x000001d94d841e86ull, 0x000001dc2c7c7df9ull, 0x000001df0b6f26dfull, 0x000001e1ea5c194eull,
        0x000001e4c943555dull, 0x000001e7a824db23ull, 0x000001ea8700aab5ull, 0x000001ed65d6c42bull,
        0x000001f044a7279dull, 0x000001f32371d51full, 0x000001f60236cccaull, 0x000001f8e0f60eb3ull,
        0x000001fbbfaf9af3ull, 0x000001fe9e63719eull, 0x000002017d1192ccull, 0x000002045bb9fe94ull,
        0x000002073a5cb50dull, 0x00000209c06e6212ull, 0x0000020cf791026aull, 0x0000020fd622997cull,
        0x00000212b07f3458ull, 0x000002159334a8d8ull, 0x0000021871b52150ull, 0x0000021b502fe517ull,
        0x0000021d6a73a78full, 0x000002210d144eeeull, 0x00000223eb7df52cull, 0x00000226c9e1e713ull,
        0x00000229a84024bbull, 0x0000022c23679b4eull, 0x0000022f64eb83a8ull, 0x000002324338a51bull,
        0x00000235218012a9ull, 0x00000237ffc1cc69ull, 0x0000023a2c3b0ea4ull, 0x0000023d13ee805bull,
        0x0000024035e9221full, 0x00000243788faf25ull, 0x0000024656b4e735ull, 0x00000247ed646bfeull,
        0x0000024c12ee3d98ull, 0x0000024ef1025c1aull, 0x00000251cf10c799ull, 0x0000025492644d65ull,
        0x000002578b1c85eeull, 0x0000025a6919d8f0ull, 0x0000025d13ee805bull, 0x0000026025036716ull,
        0x0000026296453882ull, 0x00000265e0d62b53ull, 0x00000268beb701f3ull, 0x0000026b9c92265eull,
        0x0000026d32f798a9ull, 0x00000271583758ebull, 0x000002743601673bull, 0x0000027713c5c3b0ull,
        0x00000279f1846e5full, 0x0000027ccf3d6761ull, 0x0000027e6580aecbull, 0x000002828a9e44b3ull,
        0x0000028568462932ull, 0x00000287bdbf5255ull, 0x0000028b2384de4aull, 0x0000028d13ee805bull,
        0x0000029035e9221full, 0x0000029296453882ull, 0x0000029699bdfb61ull, 0x0000029902a37aabull,
        0x0000029c54b864c9ull, 0x0000029deabd1083ull, 0x000002a20f9c0bb5ull, 0x000002a4c7605d61ull,
        0x000002a7bdbf5255ull, 0x000002a96056dafcull, 0x000002ac3daf14efull, 0x000002af1b019ecaull,
        0x000002b296453882ull, 0x000002b5d022d80full, 0x000002b8fa471cb3ull, 0x000002ba9012e713ull,
        0x000002bd6d4901ccull, 0x000002c04a796cf6ull, 0x000002c327a428a6ull, 0x000002c61a5e8f4cull,
        0x000002c8e1e891f6ull, 0x000002cbbf023fc2ull, 0x000002ce9c163e6eull, 0x000002d179248e13ull,
        0x000002d4562d2ec6ull, 0x000002d73330209dull, 0x000002da102d63b0ull, 0x000002dced24f814ull,
        };


/* compute 2^44*log2(input+1) */
u_int64_t crush_ln(unsigned int xin)
{
    unsigned int x = xin;
    int iexpon, index1, index2;
    u_int64_t RH, LH, LL, xl64, result;

    x++;

    /* normalize input */
    iexpon = 15;

    // figure out number of bits we need to shift and
    // do it in one step instead of iteratively
    if (!(x & 0x18000)) {
        int bits = __builtin_clz(x & 0x1FFFF) - 16;
        x <<= bits;
        iexpon = 15 - bits;
    }

    index1 = (x >> 8) << 1;
    /* RH ~ 2^56/index1 */
    RH = __RH_LH_tbl[index1 - 256];
    /* LH ~ 2^48 * log2(index1/256) */
    LH = __RH_LH_tbl[index1 + 1 - 256];

    /* RH*x ~ 2^48 * (2^15 + xf), xf<2^8 */
    xl64 = (int64_t)x * RH;
    xl64 >>= 48;

    result = iexpon;
    result <<= (12 + 32);

    index2 = xl64 & 0xff;
    /* LL ~ 2^48*log2(1.0+index2/2^15) */
    LL = __LL_tbl[index2];

    LH = LH + LL;

    LH >>= (48 - 12 - 32);
    result += LH;

    return result;
}


int64_t crush_straw2(uint32_t a, uint32_t  b){
    uint32_t c = 0;
    uint32_t x = 231232;
    uint32_t y = 1232;

    uint32_t hash = (1315423911) ^ a ^ b ^ c;
    crush_hashmix(a, b, hash);
    crush_hashmix(c, x, hash);
    crush_hashmix(y, a, hash);
    crush_hashmix(b, x, hash);
    crush_hashmix(y, c, hash);
    hash &= 0xffff; // hash = hash % 65536

    /*
	 * for some reason slightly less than 0x10000 produces
	 * a slightly more accurate distribution... probably a
	 * rounding effect.
	 *
	 * the natural log lookup table maps [0,0xffff]
	 * (corresponding to real numbers [1/0x10000, 1] to
	 * [0, 0xffffffffffff] (corresponding to real numbers
	 * [-11.090355,0]).
	 */

    // equal to (2^44) * log2(hash+1) - (2^44) * log2(65535+1)
    int64_t ret = crush_ln(hash) - 0x1000000000000ll;

    return ret;
}


bool compare_weighted_straws(const pair<uint64_t,string>& pairA,
                             const pair<uint64_t,string>& pairB){
    return pairA.first > pairB.first;
}

bool compare_virtual_node(const pair<uint64_t,string>& pairA,
                          const pair<uint64_t,string>& pairB){
    return pairA.first < pairB.first;
}

/*0 - utils*/
std::vector<std::string> srs_string_split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

uint32_t rc_crc32(uint32_t crc, const char* buf, size_t len) {
    static uint32_t table[256];
    static int have_table = 0;
    uint32_t rem;
    uint8_t octet;
    int i = 0;
    int j = 0;
    const char *p;
    const char *q;

    /* This check is not thread safe; there is no mutex. */
    if (have_table == 0) {
        /* Calculate CRC table. */
        for (i = 0; i < 256; i++) {
            rem = i;  /* remainder from polynomial division */
            for (j = 0; j < 8; j++) {
                if (rem & 1) {
                    rem >>= 1;
                    rem ^= 0xedb88320;
                } else
                    rem >>= 1;
            }
            table[i] = rem;
        }
        have_table = 1;
    }

    crc = ~crc;
    q = buf + len;
    for (p = buf; p < q; p++) {
        octet = *p;  /* Cast to unsigned octet. */
        crc = (crc >> 8) ^ table[(crc & 0xff) ^ octet];
    }
    return ~crc;
}

uint32_t crc32(std::string key) {
    const char *s = key.c_str();
    return rc_crc32(0, s, strlen(s));
}

int string_to_num(const std::string &str) {
    std::stringstream ss(str);
    int ret = 0;
    ss >> ret;

    return ret;
}

/*
 * return true if i<j
 * {x, 3}, {o, 2}, {*, 4} will sorted into:
 * {o, 2} < {x, 3} < {*, 4}
 */
bool cmp_weighted_keys(WeightedString i, WeightedString j) {
    if (i.weight != j.weight) {
        return i.weight < j.weight;
    }

    return i.key.compare(j.key) < 0;
}



/*1 - old*/
WeightedHashMap::WeightedHashMap(const std::vector<WeightedString>& weighted_keys) {
    _weighted_keys = weighted_keys;
    sort(_weighted_keys.begin(), _weighted_keys.end(), cmp_weighted_keys);
}

void WeightedHashMap::get_series(const std::string& key, std::vector<std::string>& series) {
    series.clear();
    uint32_t hash = crc32(key);

    std::vector<WeightedString> keys;
    for (std::vector<WeightedString>::iterator it = _weighted_keys.begin();
    it != _weighted_keys.end(); ++it)
    {
        if (it->weight <= 0) {
            continue;
        }
        keys.push_back(*it);
    }

    while (keys.size() > 0) {
        uint32_t weight_sum = 0; // weight_sum > 0
        for (std::vector<WeightedString>::iterator it = keys.begin(); it != keys.end(); ++it) {
            weight_sum += it->weight;
        }

        uint32_t select = hash % weight_sum;
        uint32_t curr = 0;
        for (std::vector<WeightedString>::iterator it = keys.begin(); it != keys.end(); ++it) {
            curr += it->weight;

            if (curr > select) {
                series.push_back(it->key);
                // remove from cadicates
                keys.erase(it);
                break;
            }
        }
    }
}

uint32_t
fnv1a(const std::string& key)
{
    unsigned char *s = (unsigned char *)key.c_str();  /* unsigned string */

    uint32_t hval = 0;

    /*
     * FNV-1a hash each octet in the buffer
     */
    for (size_t i = 0; i < key.length(); i++) {
        /* xor the bottom with the current octet */
        hval ^= (uint32_t)*s++;

        /* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
        hval *= FNV_32_PRIME;
#else
        hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
#endif
    }

    /* return our new hash value */
    return hval;
}
void WeightedHashMap::get_series_new_ch(const std::string& key, std::vector<std::string>& series) {
    series.clear();

    if(record.empty()){
        for (auto it=_weighted_keys.begin();it!=_weighted_keys.end();++it) {
            string domianKey = it->key;
            int domainWeight = it->weight;
            for(int i=0;i<domainWeight*1000;i++){
                int virtualNodeHash = fnv1a(it->key + "_" + to_string(i)) % 100000;
                record.push_back(pair<uint64_t,string>(virtualNodeHash, domianKey));
            }
        }
        sort(record.begin(), record.end(), compare_virtual_node);
    }

    unordered_map<string, int> count_map;
    for(auto it=_weighted_keys.begin();it!=_weighted_keys.end();++it) {
        count_map[it->key] = 0;
    }

    for(int i=0;i<record.size()-1;i++){
        count_map[record[i].second] += record[i+1].first - record[i].first;
    }
    count_map[record.back().second] += 100000 - record.back().first + record[0].first;


    uint32_t streamHash = fnv1a(key) % 100000;

    int virtual_node_num = record.size();
    int left = 0;
    int right = virtual_node_num;
    int mid = 0;
    int belongIndex = 0;
    if(streamHash < record[0].first || streamHash >= record.back().first){
        belongIndex = record.size() - 1;
    } else {
        while(left < right){
            cout << "binary search" << endl;
            mid = left + ((right - left) >> 1);
            cout << "left" << left << "right" << right << "mid" << mid << endl;
            if((record[mid].first <= streamHash) && (streamHash < record[mid + 1].first)){
                belongIndex = mid;
                cout << "belong to: " << belongIndex << endl;
                break;
            } else if(record[mid].first < streamHash) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }
    }


    unordered_map<string,bool> hasSelected;
    for (auto it=_weighted_keys.begin();it!=_weighted_keys.end();++it) {
        hasSelected[it->key] = false;
    }
    while(series.size()<_weighted_keys.size()){
        cout << "push series " << series.size() << endl;
        string targetKey = record[belongIndex].second;
        if(hasSelected[targetKey] == false){
            series.push_back(targetKey);
            hasSelected[targetKey] = true;
        }
        belongIndex = (belongIndex - 1) % record.size();
    }
}

void WeightedHashMap::get_series_new(const std::string& key, std::vector<std::string>& series) {
    series.clear();

    uint32_t streamHash = crc32(key);
    vector<pair<uint64_t,string>> record;
    for (auto it=_weighted_keys.begin();it!=_weighted_keys.end();++it) {
        uint32_t domainNumber = crc32(it->key);
        int64_t straw = crush_straw2(streamHash, domainNumber) / it->weight;;
        record.push_back(pair<uint64_t,string>(straw,it->key));
    }

    sort(record.begin(), record.end(), compare_weighted_straws);

    for(auto it=record.begin();it!=record.end();it++){
        series.push_back(it->second);
    }
}

const std::vector<WeightedString> WeightedHashMap::get_sorted_keys() {
    return _weighted_keys;
}

bool WeightedHashMap::get_first_choice(const std::string& key, std::string& choice) {
    std::vector<std::string> series;
    get_series(key, series);
    if (series.size() == 0) {
        return false;
    }
    choice = series[0];
    return true;
}

bool WeightedHashMap::get_first_choice_new(const std::string& key, std::string& choice) {
    std::vector<std::string> series;
    get_series_new(key, series);
    if (series.size() == 0) {
        return false;
    }
    choice = series[0];
    return true;
}

bool WeightedHashMap::get_first_choice_new_ch(const std::string& key, std::string& choice) {
    std::vector<std::string> series;
    get_series_new_ch(key, series);
    if (series.size() == 0) {
        return false;
    }
    choice = series[0];
    return true;
}

/* config item sample:
 *    domain1:30,domain2:70
 */
bool srs_parse_weighted_strings(std::string body, std::vector<WeightedString>& wss) {
    using namespace std;
    if (body.empty()) {
        return false;
    }

    vector<string> items = srs_string_split(body, ',');
    vector<WeightedString> _wss;
    for (size_t i = 0; i < items.size(); i++) {
        string item = items[i];
        if (item.empty()) {
            continue;
        }
        vector<string> toks = srs_string_split(item, ':');
        if (toks.size() != 2) {
            continue;
        }

        WeightedString ws;
        ws.key = toks[0];
        ws.weight = string_to_num(toks[1]);
        _wss.push_back(ws);
    }

    if (_wss.size() > 0) {
        wss = _wss;
        return true;
    }
    return false;
}

bool srs_choose_from_weighted_strings(const std::string &config,
                                      const std::string &key, std::string &choice) {
    std::vector<WeightedString> wss;

    if (!srs_parse_weighted_strings(config, wss) || key.empty()) {
        return false;
    }

    WeightedHashMap wm(wss);
    if (!wm.get_first_choice(key, choice)) {
        return false;
    }

    return true;
}

bool srs_choose_from_weighted_strings_new(const std::string &config,
                                          const std::string &key, std::string &choice) {
    std::vector<WeightedString> wss;

    if (!srs_parse_weighted_strings(config, wss) || key.empty()) {
        return false;
    }

    WeightedHashMap wm(wss);

    if (!wm.get_first_choice_new(key, choice)) {
        return false;
    }
//    if (!wm.bucket_straw_choice(key, choice)) {
//        return false;
//    }

    return true;
}

bool srs_choose_from_weighted_strings_new_ch(const std::string &config,
                                          const std::string &key, std::string &choice) {
    std::vector<WeightedString> wss;

    if (!srs_parse_weighted_strings(config, wss) || key.empty()) {
        return false;
    }

    WeightedHashMap wm(wss);

    if (!wm.get_first_choice_new_ch(key, choice)) {
        return false;
    }
    //    if (!wm.bucket_straw_choice(key, choice)) {
    //        return false;
    //    }

    return true;
}

/* straw */




bool WeightedHashMap::bucket_straw_choice(const string& streamName, string& choice) {
    uint32_t streamNumber = crc32(streamName);//% 100;

    string realChoice = "";
    uint64_t draw = 0;
    uint64_t high_draw = 0;
    for (auto it=_weighted_keys.begin();it!=_weighted_keys.end();++it) {
        uint32_t domainNumber = crc32(it->key);
        draw = crush_straw2(streamNumber, domainNumber);
        draw &= 0xffff;
        int64_t ln = crush_ln(draw) - 0x1000000000000ll;
        draw = ln / it->weight;
        if (draw > high_draw) {
            realChoice = it->key;
            high_draw = draw;
        }
    }
    choice = realChoice;
    return true;
}




