#CA CERT
echo "Generating CACert"
openssl genrsa -out rootkey.pem 2048
openssl req -new -x509 -key rootkey.pem -out rootcert.pem -subj '/C=ES/ST=SPain/L=Madrid/O=root/OU=CA/CN=cert/emailAddress=root@ca.cert'
cat rootcert.pem rootkey.pem > CACert.pem
rm rootcert.pem rootkey.pem

#Client valid cert
echo ""
echo "Generating client CA-signed cert"
openssl genrsa -out ckey.pem 2048
openssl req -new -key ckey.pem -out cCAsignedreq.pem -subj '/C=ES/ST=SPain/L=Madrid/O=client/OU=CASigned/CN=req/emailAddress=client@casigned.req'
openssl x509 -req -CA CACert.pem -CAserial CASerial.srl -CAcreateserial -out cCAsignedcert.pem -in cCAsignedreq.pem
cat cCAsignedcert.pem ckey.pem CACert.pem > CCASignedCert.pem
rm cCAsignedcert.pem cCAsignedreq.pem ckey.pem

#Client self-signed cert
echo ""
echo "Generating client nontrusted signed cert"
openssl genrsa -out cntkey.pem 2048
openssl req -new -x509 -key cntkey.pem -out cself.pem -subj '/C=ES/ST=SPain/L=Madrid/O=client/OU=selfSigned/CN=cert/emailAddress=client@selfsigned.cert'
cat cself.pem cntkey.pem CACert.pem > CSelfSignedCert.pem
rm cself.pem cntkey.pem

#Server valid cert
echo ""
echo "Generating server CA-signed cert"
openssl genrsa -out skey.pem 2048
openssl req -new -key skey.pem -out sCAsignedreq.pem -subj '/C=ES/ST=SPain/L=Madrid/O=server/OU=CASigned/CN=req/emailAddress=server@casigned.req'
openssl x509 -req -CA CACert.pem -CAserial CASerial.srl -CAcreateserial -out sCAsignedcert.pem -in sCAsignedreq.pem
cat sCAsignedcert.pem skey.pem CACert.pem > SCASignedCert.pem
rm sCAsignedcert.pem sCAsignedreq.pem skey.pem

#Server self-signed cert
echo ""
echo "Generating server self signed cert"
openssl genrsa -out sntkey.pem 2048
openssl req -new -x509 -key sntkey.pem -out sself.pem -subj '/C=ES/ST=SPain/L=Madrid/O=server/OU=SelfSigned/CN=cert/emailAddress=server@selfsigned.cert'
cat sself.pem sntkey.pem CACert.pem > SSelfSignedCert.pem
rm sself.pem sntkey.pem

