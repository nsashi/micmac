set(uti_image_MpDcraw_Src_Files
    ${UTI_IMAGE_MPDCRAW_DIR}/cArgMpDCRaw.cpp
    ${UTI_IMAGE_MPDCRAW_DIR}/cNChanel.cpp
    ${UTI_IMAGE_MPDCRAW_DIR}/cOneChanel.cpp
)

#SOURCE_GROUP(uti_image/MpDcraw FILES ${uti_image_MpDcraw_Src_Files})

list( APPEND uti_image_Src_Files ${uti_image_MpDcraw_Src_Files} )
