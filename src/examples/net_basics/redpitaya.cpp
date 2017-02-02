//
// Created by morrigan on 1/31/17.
//

#include <gsl/gsl_interp.h>

#include "scpi_cli.hpp"
#include "../../lib_visual/oscillosope.hpp"
#include "../../lib_visual/window.hpp"
#include "../../utils/ieee754_1985.hpp"
#include "../../lib_dsp/oscilloscope_adaptor.hpp"

//filter converting binary data to float and then cast to double
struct DOUBLE_PKT : public MESSAGE{
    vector<double> data;
    string dt;
};

using t_F_BIN_FLOAT = filter<BIN_PKT, DOUBLE_PKT>;
t_F_BIN_FLOAT::t_OUT_PTR bin_float_proc(t_F_BIN_FLOAT::t_IN_PTR&& in_msg){
    t_F_BIN_FLOAT::t_OUT_PTR out_msg(new DOUBLE_PKT);

    out_msg->data.resize(in_msg->bin_frame.size()/4);
    for(int i = 0; i < in_msg->bin_frame.size()/4; i++){
        out_msg->data[i] = (double)ieee754_1985_to_float(&in_msg->bin_frame.data()[i*4]);
    }
    out_msg->dt = move(in_msg->dt);
    return out_msg;
};

//filter - GSL interpolation
struct USR_INTERPOL_PKT : public INTERPOL_PKT{
    string dt;
};

using t_F_INTERPOLATE = filter<DOUBLE_PKT, USR_INTERPOL_PKT>;
t_F_INTERPOLATE::t_OUT_PTR interpolate_proc(t_F_INTERPOLATE::t_IN_PTR&& in_msg){
    t_F_INTERPOLATE::t_OUT_PTR out_msg(new USR_INTERPOL_PKT);

    out_msg->data_y = move(in_msg->data);
    size_t N = out_msg->data_y.size();

    //todo: transform data_x -> real time line
    out_msg->data_x.resize(N);
    for(size_t i = 0; i < N; i++) out_msg->data_x[i] = (double) i;

    out_msg->interp = shared_ptr<gsl_interp>(gsl_interp_alloc(gsl_interp_linear,N),
                                  [&](gsl_interp* p_ctx) {gsl_interp_free(p_ctx);});

    gsl_interp_init(out_msg->interp.get(), out_msg->data_x.data(), out_msg->data_y.data(),N);

    out_msg->acc = shared_ptr<gsl_interp_accel>(gsl_interp_accel_alloc(), [&](gsl_interp_accel* pa){ gsl_interp_accel_free(pa);});
    out_msg->dt = move(in_msg->dt);

    return out_msg;
}

//dummy device (just print statistics)
using t_DUMMY_DEVICE = device<USR_INTERPOL_PKT>;
bool dummy_dev_proc(t_DUMMY_DEVICE::t_IN_PTR&& in_msg){
    cout << in_msg->data_y.size() << " " << in_msg->dt << " data sample: " << in_msg->data_y[100] << endl;
    return true;
};

//oscilloscope device adaptor
using t_OSC_ADAPTOR = push_pull_osc_adaptor<USR_INTERPOL_PKT>;

int main(){

    shared_ptr<scpi_client> cli(new scpi_client("192.168.100.4","5000"));
    shared_ptr<t_F_BIN_FLOAT> f_bin_float(new t_F_BIN_FLOAT(bin_float_proc));
    shared_ptr<t_F_INTERPOLATE> f_interpolate(new t_F_INTERPOLATE(interpolate_proc));
    //shared_ptr<t_DUMMY_DEVICE> dev(new t_DUMMY_DEVICE(dummy_dev_proc));
    shared_ptr<t_OSC_ADAPTOR> dev_osc_adaptor(new t_OSC_ADAPTOR());

    cli->set_target(f_bin_float);
    f_bin_float->set_target(f_interpolate);
    f_interpolate->set_target(dev_osc_adaptor);

    dev_osc_adaptor->start();
    f_interpolate->start();
    f_bin_float->start();
    cli->start();

    Window w(unique_ptr<Oscilloscope>(new Oscilloscope(dev_osc_adaptor)));
    w.create_window(375*2,300*2);

    while(1){
        if(!w.is_running()) break;
    }

};