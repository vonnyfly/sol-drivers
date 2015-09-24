/*
 * Minimalist pseudo-device.
 * Writes a message whenever a routine is entered.
 *
 * Build the driver:
 *      cc -D_KERNEL -c scsi_pseudo.c
 *      ld -r -o scsi_pseudo scsi_pseudo.o
 * Copy the driver and the configuration file to /usr/kernel/drv:
 *      cp scsi_pseudo.conf /usr/kernel/drv
 *      cp scsi_pseudo /tmp
 *      ln -s /tmp/scsi_pseudo /usr/kernel/drv/scsi_pseudo
 * Add the driver:
 *      add_drv scsi_pseudo
 * Test (1) read from driver (2) write to driver:
 *      cat /devices/pseudo/scsi_pseudo@*
 *      echo hello > `ls /devices/pseudo/scsi_pseudo@*`
 * Verify the tests in another window:
 *      tail -f /var/adm/messages
 * Remove the driver:
 *      rem_drv scsi_pseudo
 */

#include <sys/devops.h>  /* used by dev_ops */
#include <sys/conf.h>    /* used by dev_ops and cb_ops */
#include <sys/modctl.h>  /* used by modlinkage, modldrv, _init, _info, */
                         /* and _fini */
#include <sys/types.h>   /* used by open, close, read, write, prop_op, */
                         /* and ddi_prop_op */
#include <sys/file.h>    /* used by open, close */
#include <sys/errno.h>   /* used by open, close, read, write */
#include <sys/open.h>    /* used by open, close, read, write */
#include <sys/cred.h>    /* used by open, close, read */
#include <sys/uio.h>     /* used by read */
#include <sys/stat.h>    /* defines S_IFCHR used by ddi_create_minor_node */
#include <sys/cmn_err.h> /* used by all entry points for this driver */
#include <sys/ddi.h>     /* used by all entry points for this driver */
                         /* also used by cb_ops, ddi_get_instance, and */
                         /* ddi_prop_op */
#include <sys/sunddi.h>  /* used by all entry points for this driver */
                         /* also used by cb_ops, ddi_create_minor_node, */
                         /* ddi_get_instance, and ddi_prop_op */

static int scsi_pseudo_attach(dev_info_t *dip, ddi_attach_cmd_t cmd);
static int scsi_pseudo_detach(dev_info_t *dip, ddi_detach_cmd_t cmd);
static int scsi_pseudo_getinfo(dev_info_t *dip, ddi_info_cmd_t cmd, void *arg,
    void **resultp);
static int scsi_pseudo_prop_op(dev_t dev, dev_info_t *dip, ddi_prop_op_t prop_op,
    int flags, char *name, caddr_t valuep, int *lengthp);
static int scsi_pseudo_open(dev_t *devp, int flag, int otyp, cred_t *cred);
static int scsi_pseudo_close(dev_t dev, int flag, int otyp, cred_t *cred);
static int scsi_pseudo_read(dev_t dev, struct uio *uiop, cred_t *credp);
static int scsi_pseudo_write(dev_t dev, struct uio *uiop, cred_t *credp);

/* cb_ops structure */
static struct cb_ops scsi_pseudo_cb_ops = {
    scsi_pseudo_open,
    scsi_pseudo_close,
    nodev,              /* no strategy - nodev returns ENXIO */
    nodev,              /* no print */
    nodev,              /* no dump */
    scsi_pseudo_read,
    scsi_pseudo_write,
    nodev,              /* no ioctl */
    nodev,              /* no devmap */
    nodev,              /* no mmap */
    nodev,              /* no segmap */
    nochpoll,           /* returns ENXIO for non-pollable devices */
    scsi_pseudo_prop_op,
    NULL,               /* streamtab struct; if not NULL, all above */
                        /* fields are ignored */
    D_NEW | D_MP,       /* compatibility flags: see conf.h */
    CB_REV,             /* cb_ops revision number */
    nodev,              /* no aread */
    nodev               /* no awrite */
};

/* dev_ops structure */
static struct dev_ops scsi_pseudo_dev_ops = {
    DEVO_REV,
    0,                         /* reference count */
    scsi_pseudo_getinfo,             /* no getinfo(9E) */
    nulldev,                   /* no identify(9E) - nulldev returns 0 */
    nulldev,                   /* no probe(9E) */
    scsi_pseudo_attach,
    scsi_pseudo_detach,
    nodev,                     /* no reset - nodev returns ENXIO */
    &scsi_pseudo_cb_ops,
    (struct bus_ops *)NULL,
    nodev,                     /* no power(9E) */
    ddi_quiesce_not_needed,    /* no quiesce(9E) */
};

/* modldrv structure */
static struct modldrv md = {
    &mod_driverops,     /* Type of module. This is a driver. */
    "scsi_pseudo driver",     /* Name of the module. */
    &scsi_pseudo_dev_ops
};

/* modlinkage structure */
static struct modlinkage ml = {
    MODREV_1,
    &md,
    NULL
};

/* dev_info structure */
dev_info_t *scsi_pseudo_dip;  /* keep track of one instance */


/* Loadable module configuration entry points */
int
_init(void)
{
    cmn_err(CE_NOTE, "Inside _init");
    return(mod_install(&ml));
}

int
_info(struct modinfo *modinfop)
{
    cmn_err(CE_NOTE, "Inside _info");
    return(mod_info(&ml, modinfop));
}

int
_fini(void)
{
    cmn_err(CE_NOTE, "Inside _fini");
    return(mod_remove(&ml));
}

/* Device configuration entry points */
static int
scsi_pseudo_attach(dev_info_t *dip, ddi_attach_cmd_t cmd)
{
    cmn_err(CE_NOTE, "Inside scsi_pseudo_attach");
    switch(cmd) {
    case DDI_ATTACH:
        scsi_pseudo_dip = dip;
        if (ddi_create_minor_node(dip, "0", S_IFCHR,
            ddi_get_instance(dip), DDI_PSEUDO,0)
            != DDI_SUCCESS) {
            cmn_err(CE_NOTE,
                "%s%d: attach: could not add character node.",
                "scsi_pseudo", 0);
            return(DDI_FAILURE);
        } else
            return DDI_SUCCESS;
    default:
        return DDI_FAILURE;
    }
}

static int
scsi_pseudo_detach(dev_info_t *dip, ddi_detach_cmd_t cmd)
{
    cmn_err(CE_NOTE, "Inside scsi_pseudo_detach");
    switch(cmd) {
    case DDI_DETACH:
        scsi_pseudo_dip = 0;
        ddi_remove_minor_node(dip, NULL);
        return DDI_SUCCESS;
    default:
        return DDI_FAILURE;
    }
}

static int
scsi_pseudo_getinfo(dev_info_t *dip, ddi_info_cmd_t cmd, void *arg,
    void **resultp)
{
    cmn_err(CE_NOTE, "Inside scsi_pseudo_getinfo");
    switch(cmd) {
    case DDI_INFO_DEVT2DEVINFO:
        *resultp = scsi_pseudo_dip;
        return DDI_SUCCESS;
    case DDI_INFO_DEVT2INSTANCE:
        *resultp = 0;
        return DDI_SUCCESS;
    default:
        return DDI_FAILURE;
    }
}

/* Main entry points */
static int
scsi_pseudo_prop_op(dev_t dev, dev_info_t *dip, ddi_prop_op_t prop_op,
    int flags, char *name, caddr_t valuep, int *lengthp)
{
    cmn_err(CE_NOTE, "Inside scsi_pseudo_prop_op");
    return(ddi_prop_op(dev,dip,prop_op,flags,name,valuep,lengthp));
}

static int
scsi_pseudo_open(dev_t *devp, int flag, int otyp, cred_t *cred)
{
    cmn_err(CE_NOTE, "Inside scsi_pseudo_open");
    return DDI_SUCCESS;
}

static int
scsi_pseudo_close(dev_t dev, int flag, int otyp, cred_t *cred)
{
    cmn_err(CE_NOTE, "Inside scsi_pseudo_close");
    return DDI_SUCCESS;
}

static int
scsi_pseudo_read(dev_t dev, struct uio *uiop, cred_t *credp)
{
    cmn_err(CE_NOTE, "Inside scsi_pseudo_read");
    return DDI_SUCCESS;
}

static int
scsi_pseudo_write(dev_t dev, struct uio *uiop, cred_t *credp)
{
    cmn_err(CE_NOTE, "Inside scsi_pseudo_write");
    return DDI_SUCCESS;
}
