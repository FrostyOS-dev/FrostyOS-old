#include "IDT.hpp"

extern "C" void x86_64_ISR0();
extern "C" void x86_64_ISR1();
extern "C" void x86_64_ISR2();
extern "C" void x86_64_ISR3();
extern "C" void x86_64_ISR4();
extern "C" void x86_64_ISR5();
extern "C" void x86_64_ISR6();
extern "C" void x86_64_ISR7();
extern "C" void x86_64_ISR8();
extern "C" void x86_64_ISR9();
extern "C" void x86_64_ISR10();
extern "C" void x86_64_ISR11();
extern "C" void x86_64_ISR12();
extern "C" void x86_64_ISR13();
extern "C" void x86_64_ISR14();
extern "C" void x86_64_ISR15();
extern "C" void x86_64_ISR16();
extern "C" void x86_64_ISR17();
extern "C" void x86_64_ISR18();
extern "C" void x86_64_ISR19();
extern "C" void x86_64_ISR20();
extern "C" void x86_64_ISR21();
extern "C" void x86_64_ISR22();
extern "C" void x86_64_ISR23();
extern "C" void x86_64_ISR24();
extern "C" void x86_64_ISR25();
extern "C" void x86_64_ISR26();
extern "C" void x86_64_ISR27();
extern "C" void x86_64_ISR28();
extern "C" void x86_64_ISR29();
extern "C" void x86_64_ISR30();
extern "C" void x86_64_ISR31();
extern "C" void x86_64_ISR32();
extern "C" void x86_64_ISR33();
extern "C" void x86_64_ISR34();
extern "C" void x86_64_ISR35();
extern "C" void x86_64_ISR36();
extern "C" void x86_64_ISR37();
extern "C" void x86_64_ISR38();
extern "C" void x86_64_ISR39();
extern "C" void x86_64_ISR40();
extern "C" void x86_64_ISR41();
extern "C" void x86_64_ISR42();
extern "C" void x86_64_ISR43();
extern "C" void x86_64_ISR44();
extern "C" void x86_64_ISR45();
extern "C" void x86_64_ISR46();
extern "C" void x86_64_ISR47();
extern "C" void x86_64_ISR48();
extern "C" void x86_64_ISR49();
extern "C" void x86_64_ISR50();
extern "C" void x86_64_ISR51();
extern "C" void x86_64_ISR52();
extern "C" void x86_64_ISR53();
extern "C" void x86_64_ISR54();
extern "C" void x86_64_ISR55();
extern "C" void x86_64_ISR56();
extern "C" void x86_64_ISR57();
extern "C" void x86_64_ISR58();
extern "C" void x86_64_ISR59();
extern "C" void x86_64_ISR60();
extern "C" void x86_64_ISR61();
extern "C" void x86_64_ISR62();
extern "C" void x86_64_ISR63();
extern "C" void x86_64_ISR64();
extern "C" void x86_64_ISR65();
extern "C" void x86_64_ISR66();
extern "C" void x86_64_ISR67();
extern "C" void x86_64_ISR68();
extern "C" void x86_64_ISR69();
extern "C" void x86_64_ISR70();
extern "C" void x86_64_ISR71();
extern "C" void x86_64_ISR72();
extern "C" void x86_64_ISR73();
extern "C" void x86_64_ISR74();
extern "C" void x86_64_ISR75();
extern "C" void x86_64_ISR76();
extern "C" void x86_64_ISR77();
extern "C" void x86_64_ISR78();
extern "C" void x86_64_ISR79();
extern "C" void x86_64_ISR80();
extern "C" void x86_64_ISR81();
extern "C" void x86_64_ISR82();
extern "C" void x86_64_ISR83();
extern "C" void x86_64_ISR84();
extern "C" void x86_64_ISR85();
extern "C" void x86_64_ISR86();
extern "C" void x86_64_ISR87();
extern "C" void x86_64_ISR88();
extern "C" void x86_64_ISR89();
extern "C" void x86_64_ISR90();
extern "C" void x86_64_ISR91();
extern "C" void x86_64_ISR92();
extern "C" void x86_64_ISR93();
extern "C" void x86_64_ISR94();
extern "C" void x86_64_ISR95();
extern "C" void x86_64_ISR96();
extern "C" void x86_64_ISR97();
extern "C" void x86_64_ISR98();
extern "C" void x86_64_ISR99();
extern "C" void x86_64_ISR100();
extern "C" void x86_64_ISR101();
extern "C" void x86_64_ISR102();
extern "C" void x86_64_ISR103();
extern "C" void x86_64_ISR104();
extern "C" void x86_64_ISR105();
extern "C" void x86_64_ISR106();
extern "C" void x86_64_ISR107();
extern "C" void x86_64_ISR108();
extern "C" void x86_64_ISR109();
extern "C" void x86_64_ISR110();
extern "C" void x86_64_ISR111();
extern "C" void x86_64_ISR112();
extern "C" void x86_64_ISR113();
extern "C" void x86_64_ISR114();
extern "C" void x86_64_ISR115();
extern "C" void x86_64_ISR116();
extern "C" void x86_64_ISR117();
extern "C" void x86_64_ISR118();
extern "C" void x86_64_ISR119();
extern "C" void x86_64_ISR120();
extern "C" void x86_64_ISR121();
extern "C" void x86_64_ISR122();
extern "C" void x86_64_ISR123();
extern "C" void x86_64_ISR124();
extern "C" void x86_64_ISR125();
extern "C" void x86_64_ISR126();
extern "C" void x86_64_ISR127();
extern "C" void x86_64_ISR128();
extern "C" void x86_64_ISR129();
extern "C" void x86_64_ISR130();
extern "C" void x86_64_ISR131();
extern "C" void x86_64_ISR132();
extern "C" void x86_64_ISR133();
extern "C" void x86_64_ISR134();
extern "C" void x86_64_ISR135();
extern "C" void x86_64_ISR136();
extern "C" void x86_64_ISR137();
extern "C" void x86_64_ISR138();
extern "C" void x86_64_ISR139();
extern "C" void x86_64_ISR140();
extern "C" void x86_64_ISR141();
extern "C" void x86_64_ISR142();
extern "C" void x86_64_ISR143();
extern "C" void x86_64_ISR144();
extern "C" void x86_64_ISR145();
extern "C" void x86_64_ISR146();
extern "C" void x86_64_ISR147();
extern "C" void x86_64_ISR148();
extern "C" void x86_64_ISR149();
extern "C" void x86_64_ISR150();
extern "C" void x86_64_ISR151();
extern "C" void x86_64_ISR152();
extern "C" void x86_64_ISR153();
extern "C" void x86_64_ISR154();
extern "C" void x86_64_ISR155();
extern "C" void x86_64_ISR156();
extern "C" void x86_64_ISR157();
extern "C" void x86_64_ISR158();
extern "C" void x86_64_ISR159();
extern "C" void x86_64_ISR160();
extern "C" void x86_64_ISR161();
extern "C" void x86_64_ISR162();
extern "C" void x86_64_ISR163();
extern "C" void x86_64_ISR164();
extern "C" void x86_64_ISR165();
extern "C" void x86_64_ISR166();
extern "C" void x86_64_ISR167();
extern "C" void x86_64_ISR168();
extern "C" void x86_64_ISR169();
extern "C" void x86_64_ISR170();
extern "C" void x86_64_ISR171();
extern "C" void x86_64_ISR172();
extern "C" void x86_64_ISR173();
extern "C" void x86_64_ISR174();
extern "C" void x86_64_ISR175();
extern "C" void x86_64_ISR176();
extern "C" void x86_64_ISR177();
extern "C" void x86_64_ISR178();
extern "C" void x86_64_ISR179();
extern "C" void x86_64_ISR180();
extern "C" void x86_64_ISR181();
extern "C" void x86_64_ISR182();
extern "C" void x86_64_ISR183();
extern "C" void x86_64_ISR184();
extern "C" void x86_64_ISR185();
extern "C" void x86_64_ISR186();
extern "C" void x86_64_ISR187();
extern "C" void x86_64_ISR188();
extern "C" void x86_64_ISR189();
extern "C" void x86_64_ISR190();
extern "C" void x86_64_ISR191();
extern "C" void x86_64_ISR192();
extern "C" void x86_64_ISR193();
extern "C" void x86_64_ISR194();
extern "C" void x86_64_ISR195();
extern "C" void x86_64_ISR196();
extern "C" void x86_64_ISR197();
extern "C" void x86_64_ISR198();
extern "C" void x86_64_ISR199();
extern "C" void x86_64_ISR200();
extern "C" void x86_64_ISR201();
extern "C" void x86_64_ISR202();
extern "C" void x86_64_ISR203();
extern "C" void x86_64_ISR204();
extern "C" void x86_64_ISR205();
extern "C" void x86_64_ISR206();
extern "C" void x86_64_ISR207();
extern "C" void x86_64_ISR208();
extern "C" void x86_64_ISR209();
extern "C" void x86_64_ISR210();
extern "C" void x86_64_ISR211();
extern "C" void x86_64_ISR212();
extern "C" void x86_64_ISR213();
extern "C" void x86_64_ISR214();
extern "C" void x86_64_ISR215();
extern "C" void x86_64_ISR216();
extern "C" void x86_64_ISR217();
extern "C" void x86_64_ISR218();
extern "C" void x86_64_ISR219();
extern "C" void x86_64_ISR220();
extern "C" void x86_64_ISR221();
extern "C" void x86_64_ISR222();
extern "C" void x86_64_ISR223();
extern "C" void x86_64_ISR224();
extern "C" void x86_64_ISR225();
extern "C" void x86_64_ISR226();
extern "C" void x86_64_ISR227();
extern "C" void x86_64_ISR228();
extern "C" void x86_64_ISR229();
extern "C" void x86_64_ISR230();
extern "C" void x86_64_ISR231();
extern "C" void x86_64_ISR232();
extern "C" void x86_64_ISR233();
extern "C" void x86_64_ISR234();
extern "C" void x86_64_ISR235();
extern "C" void x86_64_ISR236();
extern "C" void x86_64_ISR237();
extern "C" void x86_64_ISR238();
extern "C" void x86_64_ISR239();
extern "C" void x86_64_ISR240();
extern "C" void x86_64_ISR241();
extern "C" void x86_64_ISR242();
extern "C" void x86_64_ISR243();
extern "C" void x86_64_ISR244();
extern "C" void x86_64_ISR245();
extern "C" void x86_64_ISR246();
extern "C" void x86_64_ISR247();
extern "C" void x86_64_ISR248();
extern "C" void x86_64_ISR249();
extern "C" void x86_64_ISR250();
extern "C" void x86_64_ISR251();
extern "C" void x86_64_ISR252();
extern "C" void x86_64_ISR253();
extern "C" void x86_64_ISR254();
extern "C" void x86_64_ISR255();

void x86_64_ISR_InitializeGates() {
    x86_64_IDT_SetGate(0, (void*)x86_64_ISR0, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(1, (void*)x86_64_ISR1, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(2, (void*)x86_64_ISR2, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(3, (void*)x86_64_ISR3, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(4, (void*)x86_64_ISR4, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(5, (void*)x86_64_ISR5, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(6, (void*)x86_64_ISR6, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(7, (void*)x86_64_ISR7, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(8, (void*)x86_64_ISR8, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(9, (void*)x86_64_ISR9, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(10, (void*)x86_64_ISR10, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(11, (void*)x86_64_ISR11, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(12, (void*)x86_64_ISR12, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(13, (void*)x86_64_ISR13, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(14, (void*)x86_64_ISR14, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(15, (void*)x86_64_ISR15, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(16, (void*)x86_64_ISR16, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(17, (void*)x86_64_ISR17, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(18, (void*)x86_64_ISR18, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(19, (void*)x86_64_ISR19, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(20, (void*)x86_64_ISR20, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(21, (void*)x86_64_ISR21, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(22, (void*)x86_64_ISR22, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(23, (void*)x86_64_ISR23, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(24, (void*)x86_64_ISR24, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(25, (void*)x86_64_ISR25, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(26, (void*)x86_64_ISR26, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(27, (void*)x86_64_ISR27, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(28, (void*)x86_64_ISR28, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(29, (void*)x86_64_ISR29, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(30, (void*)x86_64_ISR30, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(31, (void*)x86_64_ISR31, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(32, (void*)x86_64_ISR32, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(33, (void*)x86_64_ISR33, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(34, (void*)x86_64_ISR34, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(35, (void*)x86_64_ISR35, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(36, (void*)x86_64_ISR36, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(37, (void*)x86_64_ISR37, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(38, (void*)x86_64_ISR38, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(39, (void*)x86_64_ISR39, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(40, (void*)x86_64_ISR40, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(41, (void*)x86_64_ISR41, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(42, (void*)x86_64_ISR42, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(43, (void*)x86_64_ISR43, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(44, (void*)x86_64_ISR44, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(45, (void*)x86_64_ISR45, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(46, (void*)x86_64_ISR46, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(47, (void*)x86_64_ISR47, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(48, (void*)x86_64_ISR48, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(49, (void*)x86_64_ISR49, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(50, (void*)x86_64_ISR50, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(51, (void*)x86_64_ISR51, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(52, (void*)x86_64_ISR52, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(53, (void*)x86_64_ISR53, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(54, (void*)x86_64_ISR54, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(55, (void*)x86_64_ISR55, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(56, (void*)x86_64_ISR56, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(57, (void*)x86_64_ISR57, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(58, (void*)x86_64_ISR58, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(59, (void*)x86_64_ISR59, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(60, (void*)x86_64_ISR60, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(61, (void*)x86_64_ISR61, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(62, (void*)x86_64_ISR62, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(63, (void*)x86_64_ISR63, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(64, (void*)x86_64_ISR64, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(65, (void*)x86_64_ISR65, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(66, (void*)x86_64_ISR66, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(67, (void*)x86_64_ISR67, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(68, (void*)x86_64_ISR68, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(69, (void*)x86_64_ISR69, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(70, (void*)x86_64_ISR70, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(71, (void*)x86_64_ISR71, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(72, (void*)x86_64_ISR72, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(73, (void*)x86_64_ISR73, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(74, (void*)x86_64_ISR74, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(75, (void*)x86_64_ISR75, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(76, (void*)x86_64_ISR76, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(77, (void*)x86_64_ISR77, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(78, (void*)x86_64_ISR78, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(79, (void*)x86_64_ISR79, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(80, (void*)x86_64_ISR80, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(81, (void*)x86_64_ISR81, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(82, (void*)x86_64_ISR82, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(83, (void*)x86_64_ISR83, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(84, (void*)x86_64_ISR84, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(85, (void*)x86_64_ISR85, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(86, (void*)x86_64_ISR86, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(87, (void*)x86_64_ISR87, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(88, (void*)x86_64_ISR88, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(89, (void*)x86_64_ISR89, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(90, (void*)x86_64_ISR90, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(91, (void*)x86_64_ISR91, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(92, (void*)x86_64_ISR92, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(93, (void*)x86_64_ISR93, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(94, (void*)x86_64_ISR94, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(95, (void*)x86_64_ISR95, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(96, (void*)x86_64_ISR96, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(97, (void*)x86_64_ISR97, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(98, (void*)x86_64_ISR98, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(99, (void*)x86_64_ISR99, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(100, (void*)x86_64_ISR100, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(101, (void*)x86_64_ISR101, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(102, (void*)x86_64_ISR102, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(103, (void*)x86_64_ISR103, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(104, (void*)x86_64_ISR104, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(105, (void*)x86_64_ISR105, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(106, (void*)x86_64_ISR106, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(107, (void*)x86_64_ISR107, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(108, (void*)x86_64_ISR108, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(109, (void*)x86_64_ISR109, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(110, (void*)x86_64_ISR110, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(111, (void*)x86_64_ISR111, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(112, (void*)x86_64_ISR112, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(113, (void*)x86_64_ISR113, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(114, (void*)x86_64_ISR114, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(115, (void*)x86_64_ISR115, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(116, (void*)x86_64_ISR116, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(117, (void*)x86_64_ISR117, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(118, (void*)x86_64_ISR118, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(119, (void*)x86_64_ISR119, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(120, (void*)x86_64_ISR120, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(121, (void*)x86_64_ISR121, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(122, (void*)x86_64_ISR122, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(123, (void*)x86_64_ISR123, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(124, (void*)x86_64_ISR124, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(125, (void*)x86_64_ISR125, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(126, (void*)x86_64_ISR126, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(127, (void*)x86_64_ISR127, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(128, (void*)x86_64_ISR128, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(129, (void*)x86_64_ISR129, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(130, (void*)x86_64_ISR130, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(131, (void*)x86_64_ISR131, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(132, (void*)x86_64_ISR132, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(133, (void*)x86_64_ISR133, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(134, (void*)x86_64_ISR134, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(135, (void*)x86_64_ISR135, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(136, (void*)x86_64_ISR136, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(137, (void*)x86_64_ISR137, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(138, (void*)x86_64_ISR138, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(139, (void*)x86_64_ISR139, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(140, (void*)x86_64_ISR140, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(141, (void*)x86_64_ISR141, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(142, (void*)x86_64_ISR142, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(143, (void*)x86_64_ISR143, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(144, (void*)x86_64_ISR144, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(145, (void*)x86_64_ISR145, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(146, (void*)x86_64_ISR146, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(147, (void*)x86_64_ISR147, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(148, (void*)x86_64_ISR148, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(149, (void*)x86_64_ISR149, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(150, (void*)x86_64_ISR150, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(151, (void*)x86_64_ISR151, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(152, (void*)x86_64_ISR152, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(153, (void*)x86_64_ISR153, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(154, (void*)x86_64_ISR154, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(155, (void*)x86_64_ISR155, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(156, (void*)x86_64_ISR156, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(157, (void*)x86_64_ISR157, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(158, (void*)x86_64_ISR158, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(159, (void*)x86_64_ISR159, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(160, (void*)x86_64_ISR160, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(161, (void*)x86_64_ISR161, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(162, (void*)x86_64_ISR162, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(163, (void*)x86_64_ISR163, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(164, (void*)x86_64_ISR164, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(165, (void*)x86_64_ISR165, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(166, (void*)x86_64_ISR166, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(167, (void*)x86_64_ISR167, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(168, (void*)x86_64_ISR168, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(169, (void*)x86_64_ISR169, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(170, (void*)x86_64_ISR170, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(171, (void*)x86_64_ISR171, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(172, (void*)x86_64_ISR172, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(173, (void*)x86_64_ISR173, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(174, (void*)x86_64_ISR174, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(175, (void*)x86_64_ISR175, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(176, (void*)x86_64_ISR176, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(177, (void*)x86_64_ISR177, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(178, (void*)x86_64_ISR178, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(179, (void*)x86_64_ISR179, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(180, (void*)x86_64_ISR180, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(181, (void*)x86_64_ISR181, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(182, (void*)x86_64_ISR182, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(183, (void*)x86_64_ISR183, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(184, (void*)x86_64_ISR184, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(185, (void*)x86_64_ISR185, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(186, (void*)x86_64_ISR186, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(187, (void*)x86_64_ISR187, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(188, (void*)x86_64_ISR188, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(189, (void*)x86_64_ISR189, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(190, (void*)x86_64_ISR190, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(191, (void*)x86_64_ISR191, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(192, (void*)x86_64_ISR192, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(193, (void*)x86_64_ISR193, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(194, (void*)x86_64_ISR194, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(195, (void*)x86_64_ISR195, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(196, (void*)x86_64_ISR196, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(197, (void*)x86_64_ISR197, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(198, (void*)x86_64_ISR198, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(199, (void*)x86_64_ISR199, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(200, (void*)x86_64_ISR200, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(201, (void*)x86_64_ISR201, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(202, (void*)x86_64_ISR202, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(203, (void*)x86_64_ISR203, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(204, (void*)x86_64_ISR204, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(205, (void*)x86_64_ISR205, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(206, (void*)x86_64_ISR206, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(207, (void*)x86_64_ISR207, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(208, (void*)x86_64_ISR208, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(209, (void*)x86_64_ISR209, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(210, (void*)x86_64_ISR210, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(211, (void*)x86_64_ISR211, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(212, (void*)x86_64_ISR212, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(213, (void*)x86_64_ISR213, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(214, (void*)x86_64_ISR214, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(215, (void*)x86_64_ISR215, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(216, (void*)x86_64_ISR216, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(217, (void*)x86_64_ISR217, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(218, (void*)x86_64_ISR218, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(219, (void*)x86_64_ISR219, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(220, (void*)x86_64_ISR220, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(221, (void*)x86_64_ISR221, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(222, (void*)x86_64_ISR222, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(223, (void*)x86_64_ISR223, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(224, (void*)x86_64_ISR224, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(225, (void*)x86_64_ISR225, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(226, (void*)x86_64_ISR226, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(227, (void*)x86_64_ISR227, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(228, (void*)x86_64_ISR228, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(229, (void*)x86_64_ISR229, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(230, (void*)x86_64_ISR230, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(231, (void*)x86_64_ISR231, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(232, (void*)x86_64_ISR232, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(233, (void*)x86_64_ISR233, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(234, (void*)x86_64_ISR234, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(235, (void*)x86_64_ISR235, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(236, (void*)x86_64_ISR236, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(237, (void*)x86_64_ISR237, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(238, (void*)x86_64_ISR238, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(239, (void*)x86_64_ISR239, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(240, (void*)x86_64_ISR240, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(241, (void*)x86_64_ISR241, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(242, (void*)x86_64_ISR242, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(243, (void*)x86_64_ISR243, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(244, (void*)x86_64_ISR244, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(245, (void*)x86_64_ISR245, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(246, (void*)x86_64_ISR246, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(247, (void*)x86_64_ISR247, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(248, (void*)x86_64_ISR248, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(249, (void*)x86_64_ISR249, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(250, (void*)x86_64_ISR250, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(251, (void*)x86_64_ISR251, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(252, (void*)x86_64_ISR252, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(253, (void*)x86_64_ISR253, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(254, (void*)x86_64_ISR254, 8, IDT_FLAG_GATE_64BIT_INT);
    x86_64_IDT_SetGate(255, (void*)x86_64_ISR255, 8, IDT_FLAG_GATE_64BIT_INT);
}