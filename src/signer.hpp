#ifndef GDK_SIGNER_HPP
#define GDK_SIGNER_HPP
#pragma once

#include "boost_wrapper.hpp"
#include "ga_wally.hpp"
#include <nlohmann/json.hpp>

namespace ga {
namespace sdk {
    class network_parameters;

    // Enum to represent the "level" of support for Liquid on an HW
    enum class liquid_support_level : uint32_t {
        none = 0, // Liquid is not supported
        lite = 1 // Liquid is supported, unblinding is done on the host
    };

    // Enum to indicate whether AE-protocol signatures are supported/mandatory
    enum class ae_protocol_support_level : uint32_t {
        none = 0, // AE signing protocol is not supported, only vanilla EC sigs
        optional = 1, // Both AE and vanilla EC sigs are supported
        mandatory = 2 // AE protocol mandatory, vanilla EC sigs not supported
    };

    //
    // Interface to signing and deriving privately derived xpub keys
    //
    class signer final {
    public:
        static const std::array<uint32_t, 1> LOGIN_PATH;
        static const std::array<uint32_t, 1> CLIENT_SECRET_PATH;
        static const std::array<unsigned char, 8> PASSWORD_SALT;
        static const std::array<unsigned char, 8> BLOB_SALT;

        // A software watch-only signer for watch-only sessions
        static std::shared_ptr<signer> make_watch_only_signer(const network_parameters& net_params);

        // A proxy for a hardware signer controlled by the caller
        static std::shared_ptr<signer> make_hardware_signer(
            const network_parameters& net_params, const nlohmann::json& hw_device);

        // A software signer that signs using a private key held in memory
        static std::shared_ptr<signer> make_software_signer(
            const network_parameters& net_params, const std::string& mnemonic_or_xpub);

        signer(const network_parameters& net_params, const nlohmann::json& hw_device);
        signer(const network_parameters& net_params, const std::string& mnemonic_or_xpub);

        signer(const signer&) = delete;
        signer& operator=(const signer&) = delete;
        signer(signer&&) = delete;
        signer& operator=(signer&&) = delete;
        virtual ~signer();

        // Return the mnemonic associated with this signer (empty if none available)
        std::string get_mnemonic(const std::string& password);

        // Returns true if if this signer produces only low-r signatures
        bool supports_low_r() const;

        // Returns true if if this signer can sign arbitrary scripts
        bool supports_arbitrary_scripts() const;

        // Returns the level of liquid support
        liquid_support_level get_liquid_support() const;

        // Returns true if if this signer can export the master blinding key
        bool supports_host_unblinding() const;

        // Returns how this signer supports the Anti-Exfil protocol
        ae_protocol_support_level get_ae_protocol_support() const;

        bool is_liquid() const;

        // Returns true if this signer is watch-only (cannot sign)
        bool is_watch_only() const;

        // Returns true if this signer is hardware (i.e. externally implemented)
        bool is_hardware() const;

        // Get the device description for this signer
        const nlohmann::json& get_device() const;

        // Get the xpub for 'm/<path>'. This should only be used to derive the master
        // xpub for privately derived master keys, since it may involve talking to
        // hardware. Use xpub_hdkeys_base to quickly derive from the resulting key.
        xpub_t get_xpub(uint32_span_t path);
        std::string get_bip32_xpub(uint32_span_t path);

        // Return the ECDSA signature for a hash using the bip32 key 'm/<path>'
        ecdsa_sig_t sign_hash(uint32_span_t path, byte_span_t hash);

        priv_key_t get_blinding_key_from_script(byte_span_t script);

        std::vector<unsigned char> get_blinding_pubkey_from_script(byte_span_t script);

        bool has_master_blinding_key() const;
        blinding_key_t get_master_blinding_key() const;
        void set_master_blinding_key(const std::string& blinding_key_hex);

    private:
        const bool m_is_main_net;
        const bool m_is_liquid;
        const unsigned char m_btc_version;
        const nlohmann::json m_device;
        std::string m_mnemonic;
        wally_ext_key_ptr m_master_key;
        boost::optional<blinding_key_t> m_master_blinding_key;
    };

} // namespace sdk
} // namespace ga

#endif
