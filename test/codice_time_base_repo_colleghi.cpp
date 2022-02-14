Encoder::Encoder(const AVCodecID codec_id, const int sample_rate, const uint64_t channel_layout,
                 const int global_header_flags, const std::map<std::string, std::string> &options)
    : Encoder(codec_id) {
    if (codec_->type != AVMEDIA_TYPE_AUDIO)
        throwRuntimeError("failed to create audio encoder (received codec ID is not of type audio)");

    codec_ctx_->sample_rate = sample_rate;
    codec_ctx_->channel_layout = channel_layout;
    codec_ctx_->channels = av_get_channel_layout_nb_channels(codec_ctx_->channel_layout);
    if (codec_->sample_fmts) codec_ctx_->sample_fmt = codec_->sample_fmts[0];
    /* for audio, the time base will be automatically set by init() */
    // codec_ctx_->time_base.num = 1;
    // codec_ctx_->time_base.den = codec_ctx_->sample_rate;

    init(global_header_flags, options);
}

Encoder::Encoder(const AVCodecID codec_id, const int width, const int height, const AVPixelFormat pix_fmt,
                 const AVRational time_base, const int global_header_flags,
                 const std::map<std::string, std::string> &options)
    : Encoder(codec_id) {
    if (codec_->type != AVMEDIA_TYPE_VIDEO)
        throwRuntimeError("failed to create video encoder (received codec ID is not of type video)");

    codec_ctx_->width = width;
    codec_ctx_->height = height;
    codec_ctx_->pix_fmt = pix_fmt;
    codec_ctx_->time_base = time_base;

    init(global_header_flags, options);
}



void Encoder::init(const int global_header_flags, const std::map<std::string, std::string> &options) {
    if (!codec_) throwLogicError("initialization failed, internal codec is null");
    if (!codec_ctx_) throwLogicError("initialization failed, internal codec ctx is null");

    if (global_header_flags & AVFMT_GLOBALHEADER) codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    av::DictionaryUPtr dict = av::map2dict(options);
    AVDictionary *dict_raw = dict.release();
    int ret = avcodec_open2(codec_ctx_.get(), codec_, dict_raw ? &dict_raw : nullptr);
    dict = av::DictionaryUPtr(dict_raw);
    if (ret) throwRuntimeError("failed to initialize Codec Context");
#if VERBOSE
    auto map = av::dict2map(dict.get());
    for (const auto &[key, val] : map) {
        std::cerr << "Encoder: couldn't find any '" << key << "' option" << std::endl;
    }
#endif
}
/******************************************************************************************/

inline DictionaryUPtr map2dict(const std::map<std::string, std::string> &map) {
    AVDictionary *dict = nullptr;
    for (const auto &[key, val] : map) {
        if (av_dict_set(&dict, key.c_str(), val.c_str(), 0) < 0) {
            if (dict) av_dict_free(&dict);
            throw std::runtime_error("Cannot set " + key + "in dictionary");
        }
    }
    return DictionaryUPtr(dict);
}
/******************************************************************************************/




