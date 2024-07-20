#!/usr/bin/env python3
import sys

# Mappers table (number -> name)
mapper_dict = {
    0: "datalatch",
    1: "mmc1",
    2: "datalatch",
    3: "datalatch",
    4: "mmc3",
    5: "mmc5",
    6: "ffe",
    7: "datalatch",
    8: "datalatch",
    9: "mmc2and4",
    10: "mmc2and4",
    11: "datalatch",
    12: "mmc3",
    13: "datalatch",
    14: "sl1632",
    15: "15",
    16: "bandai",
    17: "ffe",
    18: "18",
    19: "n106",
#   20: "",
    21: "vrc2and4",
    22: "vrc2and4",
    23: "vrc2and4",
    24: "vrc6",
    25: "vrc2and4",
    26: "vrc6",
    27: "mihunche",
    28: "28",
    29: "datalatch",
    30: "unrom512",
    31: "31",
    32: "32",
    33: "33",
    34: "34",
    35: "jyasic",
    36: "txcchip",
    37: "mmc3",
    38: "datalatch",
#   39: "",
    40: "40",
    41: "41",
    42: "42",
    43: "43",
    44: "mmc3",
    45: "mmc3",
    46: "46",
    47: "mmc3",
    48: "33",
    49: "mmc3",
    50: "50",
    51: "51",
    52: "mmc3",
    53: "supervision",
#   54: "",
#   55: "",
    56: "KS7032",
    57: "57",
    58: "addrlatch",
    59: "addrlatch",
    60: "60",
    61: "addrlatch",
    62: "62",
    63: "addrlatch",
    64: "tengen",
    65: "65",
    66: "datalatch",
    67: "67",
    68: "68",
    69: "69",
    70: "datalatch",
    71: "71",
    72: "72",
    73: "vrc3",
    74: "mmc3",
    75: "vrc1",
    76: "mmc3",
    77: "77",
    78: "datalatch",
    79: "79",
    80: "80",
    81: "81", # TODO : not working as it relies on latch.c
    82: "82",
    83: "yoko",
#   84: "",
    85: "vrc7",
    86: "datalatch",
    87: "datalatch",
    88: "88",
    89: "datalatch",
    90: "jyasic",
    91: "91",
    92: "addrlatch",
    93: "datalatch",
    94: "datalatch",
    95: "80",
    96: "96",
    97: "datalatch",
#   98: "",
    99: "99",
#   100: "",
    101: "datalatch",
#   102: "",
    103: "103",
    104: "104",
    105: "mmc1",
    106: "106",
    107: "datalatch",
    108: "108",
#   109: "",
#   110: "",
    111: "cheapocabra",
    112: "112",
    113: "datalatch",
    114: "mmc3",
    115: "mmc3",
    116: "116",
    117: "117",
    118: "mmc3",
    119: "mmc3",
    120: "120",
    121: "121",
#   122: "",
    123: "h2288",
#   124: "",
    125: "lh32",
    126: "126_422_534",
#   127: "",
    128: "128", # TODO : not working as it relies on latch.c
#   129: "",
#   130: "",
#   131: "",
    132: "txcchip",
    133: "sachen",
    134: "134",
#   135: "",
    136: "txcchip",
    137: "sachen",
    138: "sachen",
    139: "sachen",
    140: "datalatch",
    141: "sachen",
    142: "KS7032",
    143: "sachen",
    144: "datalatch",
    145: "sachen",
    146: "sachen",
    147: "txcchip",
    148: "sachen",
    149: "sachen",
    150: "sachen",
    151: "151",
    152: "datalatch",
    153: "bandai",
    154: "88",
    155: "mmc1",
    156: "156",
    157: "bandai",
    158: "tengen",
    159: "bandai",
    160: "sachen",
#   161: "",
    162: "162",
    163: "163",
    164: "164",
    165: "mmc3",
    166: "subor",
    167: "subor",
    168: "168",
#   169: "",
    170: "170",
    171: "mmc1",
    172: "txcchip",
    173: "txcchip",
    174: "174",
    175: "175",
    176: "fk23c",
    177: "177",
    178: "178",
#   179: "",
    180: "datalatch",
    181: "185",
#   182: "",
    183: "183",
    184: "datalatch",
    185: "185",
    186: "186",
    187: "187",
    188: "mmc3",
    189: "189",
    190: "190",
    191: "mmc3",
    192: "mmc3",
    193: "193",
    194: "mmc3",
    195: "195",
    196: "mmc3",
    197: "mmc3",
    198: "mmc3",
    199: "199",
    200: "addrlatch",
    201: "addrlatch",
    202: "addrlatch",
    203: "datalatch",
    204: "addrlatch",
    205: "mmc3",
    206: "206",
    207: "80",
    208: "208",
    209: "jyasic",
    210: "n106",
    211: "jyasic",
    212: "addrlatch",
    213: "addrlatch",
    214: "addrlatch",
    215: "8237",
    216: "bonza",
    217: "addrlatch",
    218: "218",
    219: "a9746",
#   220: "",
    221: "n625092",
    222: "222",
#   223: "",
    224: "268",
    225: "225",
    226: "bmc42in1r",
    227: "addrlatch",
    228: "228",
    229: "addrlatch",
    230: "230",
    231: "addrlatch",
    232: "232",
    233: "233",
    234: "234",
    235: "235",
    236: "236",
    237: "237",
    238: "603_5052",
#   239: "",
    240: "datalatch",
    241: "datalatch",
    242: "addrlatch",
    243: "sachen",
    244: "244",
    245: "mmc3",
    246: "246",
#   247: "",
#   248: "",
    249: "mmc3",
    250: "mmc3",
#   251: "",
    252: "252",
    253: "253",
    254: "mmc3",
    255: "225",
    256: "onebus",
#   257: "",
    258: "8237",
    259: "f_15",
    260: "hp10xx_hp20xx",
    261: "addrlatch",
    262: "sheroes",
    263: "kof97",
    264: "yoko",
    265: "265",
    266: "cityfighter",
    267: "267",
    268: "268",
    269: "269",
#   270: "",
    271: "datalatch",
    272: "272",
#   273: "",
    274: "bmc80013b",
#   275: "",
#   276: "",
    277: "277",
#   278: "",
#   279: "",
#   280: "",
    281: "jyasic",
    282: "jyasic",
    283: "283",
#   284: "",
    285: "datalatch",
    286: "bs_5",
    287: "411120_c",
    288: "addrlatch",
    289: "bmc60311c",
    290: "addrlatch",
    291: "291",
    292: "BMW8544",
    293: "293",
    294: "294",
    295: "jyasic",
#   296: "",
    297: "mmc1",
    298: "tf_1201",
    299: "datalatch",
    300: "addrlatch",
    301: "8157",
    302: "KS7057",
    303: "KS7057",
    304: "09_034a",
    305: "KS7031",
    306: "KS7016",
    307: "KS7037",
    308: "vrc2and4",
    309: "lh51",
    310: "310",
#   311: "",
    312: "KS7013",
    313: "resettxrom",
    314: "bmc64in1nr",
    315: "830134C",
#   316: "",
#   317: "",
#   318: "",
    319: "319",
    320: "bmc830425C4391t",
#   321: "",
    322: "bmck3033",
    323: "mmc1",
    324: "faridunrom",
    325: "mmc3",
    326: "326",
    327: "et_100",
    328: "rt_01",
    329: "edu2000",
    330: "330",
    331: "12in1",
    332: "super40in1",
    333: "8in1",
    334: "334",
    335: "bmcctc09",
    336: "datalatch",
    337: "bmcgamecard",
    338: "addrlatch",
    339: "bmck3006",
    340: "bmck3036",
    341: "addrlatch",
    342: "coolgirl",
#   343: "",
    344: "gn26",
    345: "bmcl6in1",
    346: "KS7012",
    347: "KS7030",
    348: "830118C",
    349: "addrlatch",
    350: "bmcgamecard",
    351: "351",
#   352: "",
    353: "353",
    354: "354",
    355: "3d_block",
    356: "356",
    357: "357",
    358: "jyasic",
    359: "359",
    360: "360",
    361: "mmc3",
    362: "362",
#   363: "",
    364: "364",
#   365: "",
    366: "mmc3",
#   367: "",
    368: "368",
    369: "369",
    370: "370",
#   371: "",
    372: "372",
#   373: "",
    374: "mmc1",
    375: "375",
    376: "376",
    377: "377",
#   378: "",
#   379: "",
    380: "380",
    381: "datalatch",
    382: "382",
    383: "383",
#   384: "",
    385: "addrlatch",
    386: "jyasic",
    387: "jyasic",
    388: "jyasic",
    389: "389",
    390: "390",
    391: "391",
#   392: "",
    393: "393",
    394: "jyasic",
    395: "395",
    396: "396",
    397: "jyasic",
    398: "398",
#   399: "",
#   400: "",
    401: "401",
    402: "addrlatch",
    403: "403",
    404: "mmc1",
#   405: "",
#   406: "",
#   407: "",
#   408: "",
    409: "addrlatch",
    410: "410",
    411: "411",
    412: "412",
#   413: "",
    414: "414",
    415: "datalatch",
    416: "416",
    417: "417",
#   418: "",
#   419: "",
    420: "420",
    421: "jyasic",
    422: "126_422_534",
#   423: "",
#   424: "",
#   425: "",
#   426: "",
#   427: "",
    428: "428",
    429: "datalatch",
    430: "430", # TODO : not working as it relies on mmc3.c
    431: "431",
    432: "432",
    433: "433",
    434: "434",
    435: "addrlatch",
    436: "436",
    437: "437",
    438: "438",
    439: "439",
#   440: "",
    441: "441",
#   442: "",
    443: "443",
    444: "444",
#   445: "",
#   446: "",
#   447: "",
    448: "448",
    449: "449",
#   450: "",
#   451: "",
    452: "452",
    453: "453", # TODO : not working as it relies on latch.c
    454: "454", # TODO : not working as it relies on latch.c
    455: "455",
    456: "456",
    457: "457", # TODO : not working as it relies on mmc3.c
    458: "458", # TODO : not working as it relies on mmc3.c
    459: "addrlatch",
    460: "460",
    461: "addrlatch",
#   462: "",
    463: "463",
    464: "addrlatch",
    465: "465",
    466: "466",
    467: "467",
    468: "468",
#   469: "",
    470: "inx007t",
#   471: "",
#   472: "",
#   473: "",
#   474: "",
#   475: "",
#   476: "",
#   477: "",
#   478: "",
#   479: "",
#   480: "",
#   481: "",
#   482: "",
#   483: "",
#   484: "",
#   485: "",
#   486: "",
#   487: "",
#   488: "",
#   489: "",
#   490: "",
#   491: "",
#   492: "",
#   493: "",
#   494: "",
#   495: "",
#   496: "",
#   497: "",
#   498: "",
#   499: "",
    500: "500",
    501: "501",
    502: "502",
#   503: "",
#   504: "",
#   505: "",
#   506: "",
#   507: "",
#   508: "",
#   509: "",
#   510: "",
#   511: "",
#   512: "",
    513: "SA_9602B",
#   514: "",
#   515: "",
    516: "516",
#   517: "",
    518: "dance2000",
    519: "eh8813a",
#   520: "",
    521: "dream",
    522: "KS7037",
    523: "fk23c",
    524: "vrc2and4",
    525: "vrc2and4",
    526: "bj56",
    527: "ax40g",
    528: "528",
    529: "vrc2and4",
    530: "ax5705",
#   531: "",
#   532: "",
    533: "533",
    534: "126_422_534",
    535: "lh53",
#   536: "",
#   537: "",
    538: "datalatch",
    539: "539",
    540: "359",
    541: "addrlatch",
#   542: "",
    543: "mmc1",
#   544: "",
#   545: "",
#   546: "",
#   547: "",
#   548: "",
#   549: "",
    550: "mmc1",
    551: "178",
#   552: "",
    553: "sachen",
    554: "554",
    555: "mmc3",
    556: "556",
#   557: "",
    558: "558",
}

n = len(sys.argv)

if n < 2: print("Usage :\ngen_mappers_table.py output_file.bin\n"); sys.exit(0)

# Mappers configuration
max_mappers = max(mapper_dict.keys()) + 1
entry_size = 28
output_file = sys.argv[1]

data = [mapper_dict.get(i, "") for i in range(max_mappers)]

with open(output_file, "wb") as f:
    for entry in data:
        padded_entry = entry.encode("ascii")[:entry_size].ljust(entry_size, b'\x00')
        f.write(padded_entry)

print(f"file \"{output_file}\" generated")
