# get flannel facts
# /etc/puppetlabs/code/environments/production/modules/apt/lib/facter/

require 'facter'

#Facter.add(:flannel) do
#    setcode do
#        flannel_hash = {}
#
#        flannel_hash['subnet'] = '172.16.3.4/24'
#        flannel_hash['mtu'] = '1455'
#
#        flannel_hash
#    end
#end

Facter.add(:flannel_subnet) do
    setcode do
#        Facter::Code::Execution.exec('/bin/cat /run/flannel/subnet.env | /bin/grep FLANNEL_SUBNET')
        Facter::Util::Resolution.exec('cat /run/flannel/subnet.env \
                | grep FLANNEL_SUBNET \
                | sed s/FLANNEL_SUBNET=//')
    end
end

Facter.add(:flannel_mtu) do
    setcode do
        Facter::Util::Resolution.exec('cat /run/flannel/subnet.env \
                | grep FLANNEL_MTU \
                | sed s/FLANNEL_MTU=//')
    end
end

