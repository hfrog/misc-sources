#!/bin/sh
#
# docker image repusher to local registry
# v1.0 2016-10-27 by and.ivanov@qiwi.com
#

set -e  # exit on error
set -u  # exit on unset variable

LOCAL_REGISTRY='dcr.qiwi.com'
FOLDERS='/etc/kubernetes/manifests-multi /etc/kubernetes/addons/multinode'
COMMON_SH='./kube-deploy/docker-multinode/common.sh'

#
# get variables from kube-deploy's common.sh
#
T_SH=$(dirname $COMMON_SH)/.t.sh
cat <<EOF > ${T_SH}
#!/bin/bash
source ${COMMON_SH}
kube::multinode::main
CURRENT_PLATFORM=\$(kube::helpers::host_platform)
ARCH=\${ARCH:-\${CURRENT_PLATFORM##*/}}
echo ARCH=\$ARCH
echo ETCD_VERSION=\$ETCD_VERSION
echo K8S_VERSION=\$K8S_VERSION
echo FLANNEL_VERSION=\$FLANNEL_VERSION
EOF
chmod +x ${T_SH}
eval $(${T_SH})
rm -f ${T_SH}

(
    #
    # get images from kube-deploy
    #
    grep ${LOCAL_REGISTRY} ${COMMON_SH} \
        | sed -re "s/^.*(${LOCAL_REGISTRY}\/[^[:space:]]+)[:space:]*.*$/\1/" \
        | while read s; do eval echo $s; done

    #
    # get needed images list by asking hyperkube directly
    #
    docker run --rm -ti ${LOCAL_REGISTRY}/hyperkube-amd64:${K8S_VERSION} \
        bash -c "echo $FOLDERS | tr ' ' '\n' | while read d; do \
            for f in \$d/*yaml; do \
                [ -f \$f ] && grep -i image: \$f | awk '{print \$NF}';
            done

            for f in \$d/*json; do \
                [ -f \$f ] && grep -i \\\"image\\\": \$f \
                    | awk -F\\\" '{print \$(NF-1)}';
            done
        done | grep ${LOCAL_REGISTRY}"
) | tr -d '\r' | sort | uniq | while read i; do
    echo $i | grep -qs "^${LOCAL_REGISTRY}" || { echo "error: non-local image $i"; exit 1; }
    gi=${i#${LOCAL_REGISTRY}/}  # global image
    echo $gi | grep -qs "^hyperkube-amd64" && { echo "I guess hyperkube is already in the local registry: $i"; continue; }
    echo "repushing $i ($gi)"
    docker pull $gi
    docker tag $gi $i
    docker push $i
done

