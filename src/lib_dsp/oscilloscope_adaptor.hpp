//
// Created by morrigan on 1/31/17.
//

#ifndef COMPPHYSFWK_OSCILLOSCOPE_ADAPTOR_HPP
#define COMPPHYSFWK_OSCILLOSCOPE_ADAPTOR_HPP

#include "../lib_visual/oscillosope.hpp"
#include "pipeline.hpp"

struct INTERPOL_PKT : public MESSAGE,  public OSC_INTERP_FRAME{};

template<typename T_IN>
class push_pull_osc_adaptor : public BaseNode<T_IN>, public osc_src_interface{
    static_assert(is_base_of<INTERPOL_PKT,T_IN>(),"T_IN shell be derived from INTERPOL_PKT");
public:
    using t_IN_PTR = shared_ptr<T_IN>;
    using t_Base = BaseNode<T_IN>;

    push_pull_osc_adaptor(string name = ""): t_Base(name) {};

    //shared_ptr magic, last_pkt changes by reset, so the pointer sent to another thread
    //will remain locally until the end of its scope
    const shared_ptr<OSC_INTERP_FRAME> get_frame(){ return last_pkt; }

protected:
    virtual bool process_usr_msg(t_IN_PTR&& in_msg){
        unique_lock<mutex> lck(t_Base::local_state_mtx);
        last_pkt = in_msg;
        return true;
    };
private:

    t_IN_PTR last_pkt{nullptr};
};

#endif //COMPPHYSFWK_OSCILLOSCOPE_ADAPTOR_HPP
